/**********************************************************************
Each of the companies; Qualcomm, and Lucent (hereinafter 
referred to individually as "Source" or collectively as "Sources") do 
hereby state:

To the extent to which the Source(s) may legally and freely do so, the 
Source(s), upon submission of a Contribution, grant(s) a free, 
irrevocable, non-exclusive, license to the Third Generation Partnership 
Project 2 (3GPP2) and its Organizational Partners: ARIB, CCSA, TIA, TTA, 
and TTC, under the Source's copyright or copyright license rights in the 
Contribution, to, in whole or in part, copy, make derivative works, 
perform, display and distribute the Contribution and derivative works 
thereof consistent with 3GPP2's and each Organizational Partner's 
policies and procedures, with the right to (i) sublicense the foregoing 
rights consistent with 3GPP2's and each Organizational Partner's  policies 
and procedures and (ii) copyright and sell, if applicable) in 3GPP2's name 
or each Organizational Partner's name any 3GPP2 or transposed Publication 
even though this Publication may contain the Contribution or a derivative 
work thereof.  The Contribution shall disclose any known limitations on 
the Source's rights to license as herein provided.

When a Contribution is submitted by the Source(s) to assist the 
formulating groups of 3GPP2 or any of its Organizational Partners, it 
is proposed to the Committee as a basis for discussion and is not to 
be construed as a binding proposal on the Source(s).  The Source(s) 
specifically reserve(s) the right to amend or modify the material 
contained in the Contribution. Nothing contained in the Contribution 
shall, except as herein expressly provided, be construed as conferring 
by implication, estoppel or otherwise, any license or right under (i) 
any existing or later issuing patent, whether or not the use of 
information in the document necessarily employs an invention of any 
existing or later issued patent, (ii) any copyright, (iii) any 
trademark, or (iv) any other intellectual property right.

With respect to the Software necessary for the practice of any or 
all Normative portions of the QCELP-13 Variable Rate Speech Codec as 
it exists on the date of submittal of this form, should the QCELP-13 be 
approved as a Specification or Report by 3GPP2, or as a transposed 
Standard by any of the 3GPP2's Organizational Partners, the Source(s) 
state(s) that a worldwide license to reproduce, use and distribute the 
Software, the license rights to which are held by the Source(s), will 
be made available to applicants under terms and conditions that are 
reasonable and non-discriminatory, which may include monetary compensation, 
and only to the extent necessary for the practice of any or all of the 
Normative portions of the QCELP-13 or the field of use of practice of the 
QCELP-13 Specification, Report, or Standard.  The statement contained above 
is irrevocable and shall be binding upon the Source(s).  In the event 
the rights of the Source(s) in and to copyright or copyright license 
rights subject to such commitment are assigned or transferred, the 
Source(s) shall notify the assignee or transferee of the existence of 
such commitments.
*******************************************************************/

/******************************** tdd_block_demod.c *********************************
This program implements one version of a baudot demodulator and
detector using a sum and dump approach.  The input is a file containing the 
modulated baudot waveform, 16-bit linearly quantized and sampled at 8000 
samples/sec.  The program makes decisions about blocks of samples, classifying 
each block as either:
logic 0: (-1)
logic 1: (+1)
silence: ( 0)
unknown: ( -10)
The output is an
ASCII file of block decisions.
This version uses a FIR complex demodulator for logic 0, logic 1, and a 
normalized energy detector.
*****************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "tty.h"
#include "tty_dbg.h"
#include "ops.h"

#define LOW_LEVEL_DETECT_FIX    1

#ifndef PI
#define PI              3.14159265358979323846
#endif


#define F1                      1400.0
#define F0                      1800.0
#define FS                      8000.0
#define WIN_LEN                 16      /* default window length */


double c0r[WIN_LEN], c0i[WIN_LEN];  /* demodulator arrays */
double c1r[WIN_LEN], c1i[WIN_LEN];  /* demodulator arrays */


/********************************************************************
* init_baudot_to_dit()
*********************************************************************/

void init_baudot_to_dit()
{
    short   i;
    double  energy;

	/* initialize stuff */

	for(i=0; i<WIN_LEN; ++i)
    {
        /* init filter memories and create coeff arrays */
		c0r[i] = cos(2.0 * PI * (double) i * F0 / FS);
		c0i[i] = sin(2.0 * PI * (double) i * F0 / FS);
		c1r[i] = cos(2.0 * PI * (double) i * F1 / FS);
		c1i[i] = sin(2.0 * PI * (double) i * F1 / FS);
	}

    energy = 0;
	for(i=0; i<WIN_LEN; ++i)
    {
        energy += c0r[i]*c0r[i] + c0i[i]*c0i[i];
    }
    energy = sqrt(energy);
	for(i=0; i<WIN_LEN; ++i)
    {
        c0r[i] /= energy;
        c0i[i] /= energy;
    }

    energy = 0;
	for(i=0; i<WIN_LEN; ++i)
    {
        energy += c1r[i]*c1r[i] + c1i[i]*c1i[i];
    }
    energy = sqrt(energy);
	for(i=0; i<WIN_LEN; ++i)
    {
        c1r[i] /= energy;
        c1i[i] /= energy;
    }

}

/********************************************************************
* baudot_to_dit()
*********************************************************************/

void baudot_to_dit(
    short   ditbuf[],       /* (o): Dit decisions                   */
    float   inbuf[],        /* (i): Input PCM                       */
    float   power_thresh,   /* (i): mark,space power threshold      */
    float   silence_thresh, /* (i): silence threshold               */
    short   erased)         /* (i): FER flag                        */
{
    double  sum0r, sum0i, sum1r, sum1i;  /* demodulator filter outputs */
    double  pow;                         /* energy summer */
    double  dbuf[WIN_LEN];               /* input samples, converted to double */

    int     i;
    double  pow0, pow1;
    short   dit;
    float   *p_inbuf;

    p_inbuf = inbuf;                                    OP_COUNT(MEM_MOVE);
    for( dit=0 ; dit < DITS_PER_FRAME ; dit++ )         
    {                                                   OP_COUNT(CONDITIONAL);
        if(! erased)                                
        {                                               OP_COUNT(5*MEM_INIT);
			sum0r = sum0i = sum1r = sum1i = pow = 0.0;
			for(i=0; i<WIN_LEN; ++i)
            {
                dbuf[i] = (double) p_inbuf[i];          OP_COUNT(ARRAY_STORE);
                sum0r += dbuf[i] * c0r[i];              OP_COUNT(MAC);
                sum0i += dbuf[i] * c0i[i];              OP_COUNT(MAC);
                sum1r += dbuf[i] * c1r[i];              OP_COUNT(MAC);
                sum1i += dbuf[i] * c1i[i];              OP_COUNT(MAC);
                pow += dbuf[i] * dbuf[i];               OP_COUNT(MAC);
			}
		
			/* do power detection */

            pow0 =  sum0r * sum0r + sum0i * sum0i;      OP_COUNT(2*L_MPY_LL+L_ADD+L_MOVE);
            pow1 =  sum1r * sum1r + sum1i * sum1i;      OP_COUNT(2*L_MPY_LL+L_ADD+L_MOVE);

            /******************** DEBUG ********************/
            dump_double_value( pow/(double) 0x7FFF, WIN_LEN, pow_fp);
            if( pow > 0.000001 )
            {
                if( pow1 > pow0 )
                    dump_double_value( 10000.0*pow1/pow, WIN_LEN, pow01_fp);
                else
                    dump_double_value( 10000.0*pow0/pow, WIN_LEN, pow01_fp);
            }
            else
            {
                dump_double_value( 0.0, WIN_LEN, pow01_fp);
            }
            /******************** END DEBUG ********************/
		
			/* update decision */

            ditbuf[dit] = UNKNOWN;                      OP_COUNT(ARRAY_INIT);
            pow1 = MAX(pow1,pow0);                      OP_COUNT(MAX_VAL);

#         if LOW_LEVEL_DETECT_FIX

                                                        OP_COUNT(CONDITIONAL+SUB);
            if( pow1 > pow*power_thresh )
            {
                                                        OP_COUNT(CONDITIONAL+L_ADD+L_MPY_LS);
                                                        OP_COUNT(CONDITIONAL+SUB);
                if( pow1 == pow0 )
                {
                    ditbuf[dit] = LOGIC_0;              OP_COUNT(ARRAY_INIT);
                }
                else
                {
                    ditbuf[dit] = LOGIC_1;              OP_COUNT(ARRAY_INIT);
                }
            }
            else if( (pow/(double) 0x7FFF) < silence_thresh )
            {
                                                        OP_COUNT(CONDITIONAL+SUB);
                ditbuf[dit] = SILENCE;                  OP_COUNT(ARRAY_INIT);
			} 
#         else
                                                        OP_COUNT(CONDITIONAL+SUB);
            if( (pow/(double) 0x7FFF) < silence_thresh )
            {
                                                        OP_COUNT(CONDITIONAL+SUB);
                ditbuf[dit] = SILENCE;                  OP_COUNT(ARRAY_INIT);
			} 
            else if( pow1/pow > power_thresh )
            {
                                                        OP_COUNT(CONDITIONAL+L_ADD+L_MPY_LS);
                                                        OP_COUNT(CONDITIONAL+SUB);
                if( pow1 == pow0 )
                {
                    ditbuf[dit] = LOGIC_0;              OP_COUNT(ARRAY_INIT);
                }
                else
                {
                    ditbuf[dit] = LOGIC_1;              OP_COUNT(ARRAY_INIT);
                }
            }
#         endif
		}
		else
        {
            ditbuf[dit] = ERASED;                       OP_COUNT(ARRAY_INIT);
            /*************** DEBUG ***************/
            dump_double_value( -10000.0, WIN_LEN, pow_fp); 
            /************* END DEBUG *************/
		}

        p_inbuf += WIN_LEN;                             OP_COUNT(ADD);
	}

}

	
