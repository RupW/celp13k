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
#include "typedef.h"
#include "tty.h"
#include "basic_op.h"

/***********************************************************************
*   tty_header_in()
*       Takes the header coming into the tty processing and converts
*       it to the internal format.
************************************************************************/

void tty_header_in( Word16 *counter )
{
    Word16   i;
    Word16   j;
    
  
    if( sub(*counter,TTY_SILENCE_HDR) == 0 )
    {
        *counter = TTY_SILENCE;
    }
    else if( sub(*counter,TTY_ONSET_HDR) == 0 )
    {
        *counter = TTY_ONSET;
    }
    else
    {
        j = TTY_CHAR_HDR_START;                     
        for( i=TTY_COUNTER_START ; i <= TTY_COUNTER_STOP ; i = shl(i,1) )
        {
            if( sub(*counter,j) == 0 )
            {
                *counter = i;                       
                break;
            }
            j = add(j,1);
        }
    }

} /* end tty_header_in() */


/***********************************************************************
*   tty_header_in()
*       Takes the internal counter coming out of the tty processing
*       and converts it to the external header format that is sent
*       in the speech packet.
************************************************************************/

void tty_header_out( Word16 *counter)
{
    Word16   i;
    Word16   j;

    if( sub(*counter,NON_TTY) == 0 )
    {
        return;
    }

    if( sub(*counter,TTY_ONSET) == 0 )
    {
        *counter = TTY_ONSET_HDR;                   
        return;
    }

    if( sub(*counter,TTY_SILENCE) == 0 )
    {
        *counter = TTY_SILENCE_HDR;                 
        return;
    }

    j = TTY_CHAR_HDR_START;                     
    for( i=TTY_COUNTER_START ; i <= TTY_COUNTER_STOP ; i = shl(i,1) )
    {
                                                
        if( sub(*counter,i) == 0 )
        {
            *counter = j;                       
            break;
        }
        j = add(j,1);
    }

    return;

} /* end tty_header_out() */


