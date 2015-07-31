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
#include "tty.h"
#include "ops.h"
#include "tty_dbg.h"

#define DEBUG_LEVEL     2   /* 0=OFF, -1=ALL ON, or [1-5] */
#define DEBUG(n,x)      if( (n <= DEBUG_LEVEL || DEBUG_LEVEL < 0) && tty_debug_print_flag) {x}

extern  short   bit_num;
extern  short   bit_size;
extern  short   current_counter;
extern  short   current_char;
extern  short   prev_bit;

extern  short   stop_bit_len;

static short    counter_hist[TTY_BUF_SIZE];
static short    char_hist[TTY_BUF_SIZE];

unsigned long   tty_dec_char_count = 0; /* for debugging only */

/***********************************************************************
*   init_tty_dec()
************************************************************************/
void init_tty_dec()
{
    short   i;
    /*    extern short    stop_bit_len; */
    
    init_tty_gen(NON_TTY,0);

    for( i=0 ; i < TTY_BUF_SIZE ; i++ )
    {
        char_hist[i] = 0;
        counter_hist[i] = NON_TTY;
    }

    stop_bit_len = DEFAULT_STOP_BIT_LEN;

}



/***********************************************************************
*   tty_dec()
************************************************************************/
short tty_dec(
    short   buf[],
    short   acb_gain,
    short   tty_header,
    short   tty_char,
    short   fer_flag,
    short   subframe,
    short   num_subfr,
    short   length
)
{
    short   counter;
    short   num;
    short   best_num;
    short   best_counter;
    short   best_char;
    short   i;
    short   j;


  resetOps(&tty_dec_ops);
                                                    OP_COUNT(CONDITIONAL);
  /* Convert the received header values */
  counter = tty_header;
  tty_header_in( &counter );

  if( subframe == 0 )
  {
    DEBUG(1,
        if( counter == TTY_EIGHTH_RATE )
        {
            fprintf(stdout,"1/8 Rate Packet\n");
        }
    )

    if( fer_flag == 1 )
    {
        counter_hist[0] = TTY_FER;                          OP_COUNT(MEM_INIT);
        char_hist[0] = TTY_FER;                             OP_COUNT(MEM_INIT);
    }

    /* Detect if Baudot signal is being received */
    else if( acb_gain != 0 )
    {
                                                            OP_COUNT(CONDITIONAL);
        counter_hist[0] = NON_TTY;                          OP_COUNT(MEM_INIT);
        char_hist[0] = 0;                                   OP_COUNT(MEM_INIT);
    }

    /* Sanity check the received information */
    else if( (counter == TTY_SILENCE && tty_char != TTY_SILENCE_CHAR)
            || (counter == TTY_ONSET && tty_char != TTY_ONSET_CHAR)
            || counter > TTY_COUNTER_MAX
            || counter < TTY_COUNTER_MIN
            || tty_char > TTY_CHAR_MAX
            || tty_char < TTY_CHAR_MIN )
    {
                                                            OP_COUNT(2*CONDITIONAL+8*SUB);
        counter_hist[0] = NON_TTY;                          OP_COUNT(ARRAY_INIT);
        char_hist[0] = 0;                                   OP_COUNT(ARRAY_INIT);
    }
    else
    {
                                                            OP_COUNT(3*CONDITIONAL+8*SUB);
        counter_hist[0] = counter;                          OP_COUNT(ARRAY_STORE);
        char_hist[0] = tty_char;                            OP_COUNT(ARRAY_STORE);
    }

/*----------------------------------------------------------------*/
DEBUG(1,
    fprintf(stdout,"-------------------------------\n");
    for( i=0 ; i < TTY_BUF_SIZE ; i++ )
    {
        if( counter_hist[i] == NON_TTY )
            fprintf(stdout," ( non)");
        else if( counter_hist[i] == TTY_FER )
            fprintf(stdout," ( FER)");
        else if( counter_hist[i] == TTY_SILENCE && char_hist[i] == TTY_SILENCE_CHAR)
            fprintf(stdout," silnce");
        else if( counter_hist[i] == TTY_ONSET && char_hist[i] == TTY_ONSET_CHAR)
            fprintf(stdout,"  onset");
        else if( counter_hist[i] == TTY_EIGHTH_RATE )
            fprintf(stdout," ( 8th)");
        else
            fprintf(stdout," (%d,%2d)",counter_hist[i],char_hist[i]);
    }
    fprintf(stdout,"\n");
)
/*----------------------------------------------------------------*/


    /*--------------------------------------------------------------
    * Sanity check and correct FER in middle of character
    *---------------------------------------------------------------*/

    /*********************************************************************
    *  Check for the start of a new character detected or there is an FER
    *  at the beginning or end of a character.  If it is a new character,
    *  make sure the next 8 frames have the same counter and
    *  character value.  This corrects FERS in the middle of a
    *  character.  If there is an FER at the transition of a character,
    *  then use the lookahead frames to vote on the most likely character.
    ***********************************************************************/

    if( counter_hist[CURRENT_FRAME] != counter_hist[CURRENT_FRAME+1] )
    {
        best_num = 2;                                   OP_COUNT(MEM_INIT);
        best_counter = counter_hist[CURRENT_FRAME+1];   OP_COUNT(MEM_MOVE);
        best_char = char_hist[CURRENT_FRAME+1];         OP_COUNT(MEM_MOVE);
        for( i=CURRENT_FRAME+1 ; i >= 1 ; i-- )
        {
                                                        OP_COUNT(CONDITIONAL+SUB);
            /* Exclude FERs from winning */
            if( counter_hist[i] != TTY_FER )
            {
                num = 0;                                OP_COUNT(MEM_INIT);
                for( j=CURRENT_FRAME+1 ; j >= 1 ; j-- )
                {
                                                        OP_COUNT(CONDITIONAL+2*SUB);
                    if( counter_hist[i] == counter_hist[j]
                        && char_hist[i] == char_hist[j] )
                    {
                        num++;                          OP_COUNT(ADD);
                    }
                }
                                                        OP_COUNT(CONDITIONAL+SUB);
                if( num > best_num )
                {
                    best_num = num;                     OP_COUNT(MEM_MOVE);
                    best_counter = counter_hist[i];     OP_COUNT(MEM_MOVE);
                    best_char = char_hist[i];           OP_COUNT(MEM_MOVE);
                }
            }
        }
                                                            OP_COUNT(CONDITIONAL+4*SUB);
        /* If best guess is silence */
        if( best_counter == TTY_SILENCE
            || best_counter == NON_TTY
            || best_counter == TTY_EIGHTH_RATE
            || best_counter == TTY_ONSET
            || best_counter == counter_hist[CURRENT_FRAME+1] )
        {
            /* Fix only this FER */
            counter_hist[CURRENT_FRAME] = best_counter; OP_COUNT(ARRAY_STORE);
            char_hist[CURRENT_FRAME] = best_char;       OP_COUNT(ARRAY_STORE);
        }
        else
        {
            if( counter_hist[CURRENT_FRAME+1] == NON_TTY )
            {

                DEBUG(1,fprintf(stdout,"tty_dec(): Force NON_TTY, silence missing.\n");)
                counter_hist[CURRENT_FRAME] = NON_TTY;          OP_COUNT(ARRAY_STORE);
                char_hist[CURRENT_FRAME] = 0;                   OP_COUNT(ARRAY_STORE);
            }
            else
            {
                tty_dec_char_count++;       /* for debugging */
                DEBUG(1,fprintf(stdout,"tty_dec(): %lu: Got new character (%2d,%2d)\n",
                    tty_dec_char_count,best_counter,best_char);)

                /* else fix a character's worth of frames */
                for( i=CURRENT_FRAME ; i > CURRENT_FRAME-7 ; i-- )
                {
                    counter_hist[i] = best_counter;         OP_COUNT(ARRAY_STORE);
                    char_hist[i] = best_char;               OP_COUNT(ARRAY_STORE);
                }
            }
        }
                                                            OP_COUNT(CONDITIONAL+SUB);
    }
                                                            OP_COUNT(CONDITIONAL+SUB);
    /*****************************************************************
    * if eighth rate, keep doing what was done in the last frame
    ******************************************************************/

    if( counter_hist[CURRENT_FRAME] == TTY_EIGHTH_RATE )
    {
        counter_hist[CURRENT_FRAME] = counter_hist[CURRENT_FRAME+1];    OP_COUNT(ARRAY_STORE);
        char_hist[CURRENT_FRAME] = char_hist[CURRENT_FRAME+1];          OP_COUNT(ARRAY_STORE);
    }


    /*****************************************************************
    * If NON_TTY is coming and there is trailing silence, get rid
    * of the silence.  This helps return to NON_TTY if there was
    * a false alarm
    ******************************************************************/

                                                            OP_COUNT(CONDITIONAL+SUB);
    if( counter_hist[0] == NON_TTY )
    {
        for( i=1 ; i <= CURRENT_FRAME ; i++ )
        {
                                                            OP_COUNT(CONDITIONAL+SUB);
            if( counter_hist[i] == TTY_SILENCE )
            {
                counter_hist[i] = NON_TTY;                  OP_COUNT(MEM_INIT);
            }
            else
            {
                break;
            }
        }
    }


    /*****************************************************************
    * Anticipate the transition from NON_TTY to TTY_SILENCE by
    * looking ahead when the mode is NON_TTY and vote on the presence
    * of TTY_SILENCE.  If there is a majority of TTY_SILENCE, change
    * the rest of the history buffer from NON_TTY to TTY_SILENCE.
    ******************************************************************/


    /* Count the number of silence frames */
    num = 0;                                              OP_COUNT(MEM_INIT);
    for( i=0 ; i <= CONVERT_TO_SILENCE_THRESH+1 ; i++ )
    {
                                                            OP_COUNT(CONDITIONAL+SUB);
        if( counter_hist[CONVERT_TO_SILENCE_THRESH+1-i] == TTY_SILENCE )
        {
            num++;                                        OP_COUNT(ADD);
        }
        else if( counter_hist[CONVERT_TO_SILENCE_THRESH+1-i] == NON_TTY )
        {
            /* Start over if NON_TTY in middle of silence */
            num = 0;
        }
    }
                                                            OP_COUNT(CONDITIONAL+SUB);
    /* Convert NON_TTY, 8th rate, and FERs to silence */
    if( num >= CONVERT_TO_SILENCE_THRESH )
    {
        for( i=0 ; i <= CURRENT_FRAME ; i++ )
        {
                                                            OP_COUNT(CONDITIONAL+3*SUB);
            if( counter_hist[i] == NON_TTY
                || counter_hist[i] == TTY_EIGHTH_RATE
                || counter_hist[i] == TTY_FER
              )
            {
                counter_hist[i] = TTY_SILENCE;              OP_COUNT(MEM_INIT);
                char_hist[i] = TTY_SILENCE_CHAR;            OP_COUNT(MEM_INIT);
            }
        }
    }

/*----------------------------------------------------------------*/
DEBUG(1,
    for( i=0 ; i < TTY_BUF_SIZE ; i++ )
    {
        if( counter_hist[i] == NON_TTY )
            fprintf(stdout," ( non)");
        else if( counter_hist[i] == TTY_FER )
            fprintf(stdout," ( FER)");
        else if( counter_hist[i] == TTY_SILENCE && char_hist[i] == TTY_SILENCE_CHAR)
            fprintf(stdout," silnce");
        else if( counter_hist[i] == TTY_ONSET && char_hist[i] == TTY_ONSET_CHAR)
            fprintf(stdout,"  onset");
        else if( counter_hist[i] == TTY_EIGHTH_RATE )
            fprintf(stdout," ( 8th)");
        else
            fprintf(stdout," (%d,%2d)",counter_hist[i],char_hist[i]);
    }
    fprintf(stdout,"__\n");

    if( counter_hist[CURRENT_FRAME] == TTY_FER )
    {
        fprintf(stdout,"Uncorrected FER.\n");
        counter_hist[CURRENT_FRAME] = TTY_SILENCE;
        char_hist[CURRENT_FRAME] = TTY_SILENCE_CHAR;
    }
    fprintf(stdout,"-------------------------------\n");
)
/*----------------------------------------------------------------*/


  } /* end if(subframe == 0) */
  else
  {
                                                        OP_COUNT(CONDITIONAL+ADD);
    /* If acb_gain is non-zero at any time in the frame, declare NON_TTY */
    if( acb_gain != 0 && !fer_flag )
    {
        counter_hist[0] = NON_TTY;                      OP_COUNT(ARRAY_INIT);
        char_hist[0] = 0;                               OP_COUNT(ARRAY_INIT);
    }
  }

DEBUG(1,
  if( (counter_hist[CURRENT_FRAME] == TTY_SILENCE && char_hist[CURRENT_FRAME] != TTY_SILENCE_CHAR)
      || (counter_hist[CURRENT_FRAME] == TTY_ONSET && char_hist[CURRENT_FRAME] != TTY_ONSET_CHAR)
      || counter_hist[CURRENT_FRAME] >= TTY_FER
    )
  {
        fprintf(stdout,"tty_dec(): Illegal value being generated. (%d,%2d)\n",
                counter_hist[CURRENT_FRAME],char_hist[CURRENT_FRAME]);
  }
)

  /* Generate the Baudot signal */
  i = tty_gen( buf, length, subframe, num_subfr, counter_hist, char_hist );

  updateOps(&tty_dec_ops);

  return(i);

} /* end tty_dec() */

            


