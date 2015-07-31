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


#include <stdio.h>
#include <string.h>
#include <math.h>
#include "typedef.h"
#include "tty.h"
#include "basic_op.h"

#ifndef PI
#define PI              3.14159265358979323846
#endif

#ifndef MIN
#define MIN(x,y)        (((x)<(y)) ? (x) : (y))
#endif

Word16 prev_freq = MARK_FREQ;

/*------------------------------------------------------------------
*       cos(wk) = 2*cos(w)*cos(w(k-1)) - cos(w(k-2))
*
*  Which is of the form:
*
*       y[k] = x*y[k-1] - y[k-2]
*
* Initial conditions:
*       param[0] = 32767*cos(2*PI*freq/8000)            (2*cos(w) in Q14)
*       param[1] = y[-2] = 16384;                       (cos(0)   in Q14)
*       param[2] = y[-1] = 16384*cos(2*PI*freq/8000)    (cos(w)   in Q14)
*------------------------------------------------------------------*/

void tone_gen(
    Word16      outbuf[],       /* (o) output in Q14                    */
    Word16      freq,           /* (i) desired freq in Hz               */
    Word16      volume,         /* (i) gain in Q14                      */
    Word16      len,            /* (i) # of samples to generate         */
    Word16      param[])        /* (i) param[0] = 2*cos(2*PI*freq/8000) */
                                /* (i) param[1] = y[k-2]                */
                                /* (i) param[2] = y[k-1]                */
{

    Word16  i;
    Word32  accA;


    /*** Generate first 2 samples using previous history ***/

    for( i=0 ; i < MIN(len,2) ; i++ )
    {                                                       
        /* outbuf[i] = (((param[0]*param[2])>>14) - param[1]);*/
        accA = L_mult(param[0],param[2]);
        accA = L_shr(accA,15);          /* shr 15 because L_mult adds shl */
        outbuf[i] = extract_l(accA);
        outbuf[i] = sub(outbuf[i],param[1]);

        param[1] = param[2];                               
        param[2] = outbuf[i];                              
    }


    /*** Generate the rest of the samples ***/

    for( i=2 ; i < len ; i++ )
    {
        /* outbuf[i] = (((param[0]*outbuf[i-1])>>14) - outbuf[i-2]);*/
        accA = L_mult(param[0],outbuf[i-1]);
        accA = L_shr(accA,15);          /* shr 15 because L_mult adds shl */
        outbuf[i] = extract_l(accA);
        outbuf[i] = sub(outbuf[i],outbuf[i-2]);
    }

    /*** Update parameters ***/
                                                           
    if( sub(len,2) >= 0 )
    {
        param[1] = outbuf[len-2];                          
        param[2] = outbuf[len-1];                           
    }

    /*** Volume Control ***/

    for( i=0 ; i < len ; i++ )
    {
        accA = L_mult(outbuf[i],volume);    /* Q14*Q14 */
        accA = L_shl(accA,1);
        outbuf[i] = extract_h(accA);
    }

}

/*-------------------------------------------------------------------
*       init_tone_gen()
*--------------------------------------------------------------------*/

void init_tone_gen(
    Word16      param[],        /* (o) tone generation parameters       */
    Word16      freq )          /* (i) Desired freq as 2*cos(w)         */
{

    param[0] = freq;    
    param[1] = 16384;                                           
    param[2] = shr(param[0],1);

} /* end init_tone_gen() */
