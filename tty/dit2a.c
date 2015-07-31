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
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "typedef.h"
#include "tty.h"
#include "basic_op.h"

#include "tty_dbg.h"

#define DEBUG_LEVEL     1   /* 0=OFF, -1=ALL ON, or [1-5] */
#define DEBUG(n,x)      if( (n <= DEBUG_LEVEL || DEBUG_LEVEL < 0) && tty_debug_print_flag) {x}

#define MIN_TTY_ONSET_FRAMES    4   /* Minimum times to transmit TTY_ONSET */

#define FRAMING_HANGOVER    12      /* # of frames from the start
                                    *  of framing a character 
                                    *  to still output that  
                                    *  character's info
                                    */


void FlushBuffer(Word16 *inbuf,Word16 bufLen,Word16 fillValue);

Word16    framingCount;
Word16    ttyState;
Word16    last_char_counter;
Word16    first_tty_char_flag;

Word16    last_tty_char;

Word16  tty_bit_hist[TTY_BIT_HIST_LEN];
Word16  tty_bit_len_hist[TTY_BIT_HIST_LEN];

unsigned long   dit2a_char_count = 0;   /* for debugging only */

/*********************************************************************
*   init_dit_to_ascii()
**********************************************************************/

void init_dit_to_ascii()
{
	/* initialize stuff */

    ttyState = NON_TTY_MODE;
    framingCount = -MIN_TTY_ONSET_FRAMES;
    last_tty_char = -1;

    last_char_counter = TTY_COUNTER_STOP;
    first_tty_char_flag = 0;

    FlushBuffer(tty_bit_hist,TTY_BIT_HIST_LEN,UNKNOWN);
    FlushBuffer(tty_bit_len_hist,TTY_BIT_HIST_LEN,0);

} 
	
/*********************************************************************
*   dit_to_ascii()
**********************************************************************/

void dit_to_ascii(
    Word16  *tty_char,      /* (i/o): prev/new character decision   */
    Word16  *char_counter,  /* (i/o): prev/new character counter    */
    Word16  *tty_baud_rate, /* (i/o): prev/new character counter    */
    Word16  ditbuf[])     /* (i): dits for current frame          */
{
    
    Word16  temp;
    Word16  bit_index;


    DEBUG(2,fprintf(stdout,"--------------------------------------\n");)

    if( sub(framingCount,FRAMING_HANGOVER) >= 0 )
    {
        last_tty_char = -1;
        framingCount = 0;
    }

    bit_index = 0;
    while( sub(bit_index,DITBUF_LEN-MIN_BIT_LEN+1) < 0 )
    {
        temp = get_tty_bit( tty_bit_hist,
                            tty_bit_len_hist,
                            &bit_index,
                            ditbuf,
                            *tty_baud_rate );

        DEBUG(2,
        {
          int   i;
            fprintf(stdout,"\ntty_bit_hist[]: ");
            for( i=0 ; i < TTY_BIT_HIST_LEN ; i++ )
            {
                fprintf(stdout," %3d",tty_bit_hist[i]);
            }
            fprintf(stdout,"\n                ");
            for( i=0 ; i < TTY_BIT_HIST_LEN ; i++ )
            {
                fprintf(stdout," %3d",tty_bit_len_hist[i]);
            }
            fprintf(stdout,"\n\n");
        }
        )


        temp = get_tty_char( tty_char, tty_bit_hist, tty_bit_len_hist );

        get_tty_state( &ttyState,
                       first_tty_char_flag,
                       tty_bit_hist,
                       tty_bit_len_hist );

                                                        
        if( temp != 0 )
        {
            if( framingCount > 0 )
            {
                framingCount = 0;                       
            }
            first_tty_char_flag = 1;                    

            /* Increment to next counter value (by shifting) */
            *char_counter = shl(last_char_counter,1);     
                                                        
            if( sub((*char_counter),TTY_COUNTER_STOP) > 0 )
            {
                *char_counter = TTY_COUNTER_START;      
            }
            last_char_counter = *char_counter;          
            last_tty_char = *tty_char;

            tty_rate( tty_baud_rate, tty_bit_len_hist );

            ++dit2a_char_count;

            DEBUG(1,fprintf(stdout,"dit2a: %lu: Regained char = %2d  counter = %d  rate = %d\n",
                dit2a_char_count, *tty_char, *char_counter, *tty_baud_rate );)
        }

    } /* end while() */

    if( last_tty_char < 0 ) /* if a character is not ready */
    {
        if( sub(ttyState,NON_TTY_MODE) == 0 )
        {
                                                        
            *char_counter = NON_TTY;                    
            *tty_char = 0;                              
            framingCount = -MIN_TTY_ONSET_FRAMES;
        }
        else /* ttyState == TTY_ONSET */
        {
            /***
            **** FIX: changed from SILENCE to ONSET so that encoder
            **** forces the rate to full rate - sab
            ***/
            
            *char_counter = TTY_ONSET;                  
            *tty_char = TTY_ONSET_CHAR;                 
            if( framingCount < 0 )
            {
                framingCount = add(framingCount,1);
            }
        }
    }
    else if( framingCount < 0 ) /* if a character is ready but not enough TTY_ONSET messages */
    {
        *char_counter = TTY_ONSET;                  
        *tty_char = TTY_ONSET_CHAR;                 
        framingCount = add(framingCount,1);
    }
    else
    {
        *char_counter = last_char_counter;
        *tty_char = last_tty_char;
        framingCount = add(framingCount,1);
    }


    DEBUG(3,fprintf(stdout,"ttyState = %2d  framingCount = %2d  counter = %2d  char = %2d\n",
        ttyState,framingCount,*char_counter,*tty_char);)

    
} /* end dit_to_ascii() */



/*********************************************************************
*   FlushBuffer()
**********************************************************************/

void FlushBuffer(Word16 *inbuf,Word16 bufLen,Word16 fillValue)
{
    Word16  i;

	for(i=0; i<bufLen; ++i)
    {
        inbuf[i] = fillValue;                               
    }
}


