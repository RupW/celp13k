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


/******************************** bdo2dit.c *********************************
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
#include <string.h>
#include <math.h>
#include "typedef.h"
#include "tty.h"
#include "tty_dbg.h"
#include "basic_op.h"
#include "basic_op40.h"


#ifndef PI
#define PI              3.14159265358979323846
#endif

#define WIN_LEN         16      /* default window length */
#define HEADROOM        3
#define TTY_SILENCE_THRESH     3        /* 300 */
#define TTY_POWER_THRESH       12500    /* 9830 */

Word16 tty_inband_filter(Word16*,Word16*,Word16);
Word16 tty_inband_energy(Word16*,Word16*,Word16*,Word16);

/* coeff arrays (stored as ROM tables) */
Word16  r_c0r[WIN_LEN] = {
  32767,   5126, -31164, -14876,  26510,  23170, -19261, -29197,
  10126,  32365,      0, -32365, -10126,  29197,  19261, -23170
};

Word16  r_c1r[WIN_LEN] = {
  32767,  14876, -19261, -32365, -10126,  23170,  31164,   5126,
 -26510, -29197,      0,  29197,  26510,  -5126, -31164, -23170
};

Word16  r_c0i[WIN_LEN] = {
      0,  32365,  10126, -29197, -19261,  23170,  26510, -14876,
 -31164,   5126,  32767,   5126, -31164, -14876,  26510,  23170
};

Word16  r_c1i[WIN_LEN] = {
      0,  29197,  26510,  -5126, -31164, -23170,  10126,  32365,
  19261, -14876, -32768, -14876,  19261,  32365,  10126, -23170
};

/********************************************************************
* baudot_to_dit()
*********************************************************************/

void baudot_to_dit(
    Word16   ditbuf[],       /* (o): Dit decisions                   */
    Word16   inbuf[]        /* (i): Input PCM                       */
)   
{
    Word40   accA;
    Word40   accB;
    Word16   pow;
    Word16   pow0;
    Word16   pow1;
    Word16   pow_shift;
    Word16   *iP;    /* use arX register in DSP */
    Word16   *dP;    /* use arX register in DSP */

    /* int     i; */
    Word16   i; 
    Word16   dit;
    Word16   *p_inbuf;

    
    p_inbuf = inbuf;                                    
    dP = ditbuf;
    for( dit=0 ; dit < DITS_PER_FRAME ; dit++ )         
    {                                                 
        /* Compute pow */
        iP = p_inbuf;
        accA = (Word40) 0.0;
        for(i=0; i<WIN_LEN; ++i)
        {
            accA = L_mac40(accA, *iP, *iP);
            iP++;
        }
        accA = L_shr40(accA,HEADROOM);
        pow_shift = norm32_l40(accA);         /* compute shift value */
        accB = (Word40) pow_shift;
        accB = L_shr40(accB,1);
        i = extract_l40(accB);
        i = sub(i,HEADROOM);

        /* Keep pow_shift within (-15, 16) range to save cycles in DSP */
        pow_shift = extract_l40(accB);
        accA = L_shl40(accA, pow_shift);    /* shift by 2*pow_shift */
        accA = L_shl40(accA, pow_shift);

        accA = L_sat32_40(accA);
        pow = round((Word32) accA);        
	
        /* Compute pow0 */
        pow0 = tty_inband_energy( p_inbuf, r_c0r, r_c0i, i );
	
        /* Compute pow1 */
        pow1 = tty_inband_energy( p_inbuf, r_c1r, r_c1i, i );
	
        /* update decision */
	
        *dP = UNKNOWN;                      
        pow1 = MAX(pow1,pow0);
	
        /* if( pow1 > (pow * power_thresh) ) */
        accA = (Word40) L_mult(pow,TTY_POWER_THRESH);

        if( L_sub40( accA, L_deposit_h(pow1)) < 0)
        {
            if( sub(pow1,pow0) == 0 )
            {
                *dP = LOGIC_0;              
            } 
            else
            {
                *dP = LOGIC_1;              
            }
        }

#if MIN_INPUT_LEVEL_FIX
        /*** Do not detect TTY if input below a minimum level ***/

        accA = L_deposit_h(pow);
        accA = L_shr40(accA,pow_shift);     /* un-normalize */
        accA = L_shr40(accA,pow_shift); 
        accB = L_deposit_l(TTY_MIN_INPUT_THRESH);
	    
        /*dump_short_value(S_get_hi(A_shl_a(accA,16)),16,pow_fp);*/
        dump_short_value(round(L_sat32_40(L_shl40(L_sub40(accA,accB),16))),16,pow_fp);

        if (L_sub40(accA,accB) < 0)
        {
            *dP = UNKNOWN;
        } 
#endif /* check for min input level */

        p_inbuf += WIN_LEN;                             
        dP++;                                           
      }

    
} /* end baudot_to_dit() */


/********************************************************************
* tty_inband_energy()
*********************************************************************/
Word16 tty_inband_energy(
    Word16   inbuf[],
    Word16   r_filter[],
    Word16   i_filter[],
    Word16   shift
)
{
  Word40  accA;
  Word16  sum_r;
  Word16  sum_i;
  
  
  sum_r = tty_inband_filter( inbuf, r_filter, shift );
  sum_i = tty_inband_filter( inbuf, i_filter, shift );
  
  /* inband_energy = sum_r^2 + sum_i^2 */
  accA = (Word40) 0.0;
  accA = L_mac40(accA, sum_r, sum_r);
  accA = L_mac40(accA, sum_i, sum_i);
  accA = L_shr40(accA, 1);
  accA = L_sat32_40(accA);

  
  return( round((Word32) accA) );
  
} /* end tty_inband_energy() */


/********************************************************************
 * tty_inband_filter()
 *********************************************************************/

Word16  tty_inband_filter(
    Word16  inbuf[],
    Word16  filter[],
    Word16  shift
)
{
  Word40  accA;
  Word16  i;
  Word16  *iP;
  Word16  *rP;
  
  
  iP = inbuf;
  rP = filter;
  accA = (Word40) 0.0;
  
  for(i=0; i<WIN_LEN; ++i)
    {
      accA = L_mac40(accA, *iP, *rP);
      iP++;
      rP++;
    }
  
  accA = L_shl40(accA, shift);  /* normalize */
  accA = L_sat32_40(accA);
  i = round((Word32) accA);        

  
  
  return(i);
  
} /* end tty_inband_filter() */




	
