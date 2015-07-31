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

/*************************************************************************
* tty_rate()
*   After a character is framed, tty_rate() decides if the character is
*   50 baud or 45.45 baud Baudot code.
**************************************************************************/

#include <stdio.h>
#include "typedef.h"
#include "tty.h"
#include "basic_op.h"


#define TTY_RATE_THRESH         63
#define TTY_RATE_THRESH_2       71
#define MAX_TTY_BAUD_COUNT      3


void tty_rate(
    Word16   *tty_baud_rate,     /* (i/o): prev/new tty baud rate    */
    Word16   tty_bit_len_hist[]  /* (i): bit len buffer              */
)
{
    Word16   i;
    Word16   char_len;

    static Word16    tty_baud_count = MAX_TTY_BAUD_COUNT;


    char_len = 0;                                           
    for( i=START_BIT_IDX ; i < STOP_BIT_IDX ; i++ )
    {
        char_len = add(char_len,tty_bit_len_hist[i]);
    }

    if( sub(char_len,TTY_RATE_THRESH) >= 0 )
    {
        i = TTY_45_BAUD;                                    

	/* 
	* Add the stop bit length and test the total
	* length.  If it is much too short for 45.45
	* baud, change it to 50 baud.
	*/
        char_len = add(char_len,tty_bit_len_hist[STOP_BIT_IDX]);
    if( sub(char_len,TTY_RATE_THRESH_2) < 0 )
	{
	    i = TTY_50_BAUD;
	}
    }
    else
    {
        i = TTY_50_BAUD;                                
    }
                                                        
    if( sub(i,*tty_baud_rate) == 0 )
    {
        tty_baud_count = MAX_TTY_BAUD_COUNT;            
    }
    else
    {
        /*
        *  Switch baud rates if the new rate occured
        *  3 characters in a row.
        */

        tty_baud_count = sub(tty_baud_count,1);
                                                            
        if( tty_baud_count <= 0 )
        {            
            *tty_baud_rate = i;                             
        }
    }


} /* end tty_rate() */


