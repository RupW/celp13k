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
#include "tty.h"
#include "ops.h"
#include "tty_dbg.h"

#define ONE_STOP_BIT_FIX        1

#define DEBUG_LEVEL     1   /* 0=OFF, -1=ALL ON, or [1-5] */
#define DEBUG(n,x)      if( (n <= DEBUG_LEVEL || DEBUG_LEVEL < 0) && tty_debug_print_flag) {x}


#define STOP_BIT_MASK       0x00C0
#define DATA_BIT_MASK       0x001F

#define STOP_BIT_NUM        6
#define MARK_HOLD_BIT_NUM   7

static short    tone_param[3];

short    bit_num;
short    bit_size;
short    current_counter;
short    current_char;
short    prev_bit;


short           stop_bit_len;       /* initialized in init_tty_dec() & init_tty_rxtx() */

/*------------------------------------------------------------------
*                       init_tty_gen()
*-------------------------------------------------------------------*/
void init_tty_gen(short counter, short tty_char)
{

    bit_size = MIN_START_BIT_LEN;                           OP_COUNT(MEM_INIT);
    bit_num = 0;                                            OP_COUNT(MEM_INIT);
    prev_bit = MARK_FREQ;                                   OP_COUNT(MEM_MOVE);
    current_counter = counter;                              OP_COUNT(MEM_MOVE);
    current_char = ((tty_char & DATA_BIT_MASK) << 1) | STOP_BIT_MASK;      OP_COUNT(AND+SHIFT+OR);

}

/*------------------------------------------------------------------
*                       tty_gen()
*-------------------------------------------------------------------*/
short tty_gen(
    short   outbuf[],           /* (o): output buffer                   */
    short   length,             /* (i): length of buffer                */
    short   subframe,           /* (i): subframe counter                */
    short   num_subfr,          /* (i): number of subframes/frame       */
    short   counter_hist[],     /* (i/o): tty counter history           */
    short   char_hist[]         /* (i/o): tty character history         */
)
{
    short   i;
    short   num;
    short   bit;
    short   *p_buf;
    short   write_flag;


    bit = 0;    /* eliminates compiler warning */
                                                            OP_COUNT(CONDITIONAL+SUB);
    /* Change onset to silence */
    if( counter_hist[CURRENT_FRAME] == TTY_ONSET )
    {
        counter_hist[CURRENT_FRAME] = TTY_SILENCE;          OP_COUNT(ARRAY_INIT);
        char_hist[CURRENT_FRAME] = TTY_SILENCE_CHAR;        OP_COUNT(ARRAY_INIT);
    }

                                                            OP_COUNT(CONDITIONAL+3*SUB);
    /* If transition from silence to anything else */
    if( ( current_counter == TTY_SILENCE
          || current_counter == NON_TTY )
        && counter_hist[CURRENT_FRAME] != TTY_SILENCE
      )
    {
        init_tty_gen( counter_hist[CURRENT_FRAME],
                      char_hist[CURRENT_FRAME]);
    }

    /* else if transition from NON_TTY to TTY_SILENCE */

    else if( current_counter == NON_TTY
             && counter_hist[CURRENT_FRAME] == TTY_SILENCE
           )
    {
                                                            OP_COUNT(CONDITIONAL+2*SUB);
        init_tty_gen( TTY_SILENCE, TTY_SILENCE_CHAR);
    }

    /* else if a new character is coming before the current one is finished */
    else if( subframe == 0
             && current_counter != counter_hist[CURRENT_FRAME]
             && counter_hist[CURRENT_FRAME] >= TTY_COUNTER_START
             && counter_hist[CURRENT_FRAME] <= TTY_COUNTER_STOP
           )
    {
        if( bit_num == MARK_HOLD_BIT_NUM )
        {
            /* Cut short the mark hold tone to start new character */
            init_tty_gen( counter_hist[CURRENT_FRAME],
                          char_hist[CURRENT_FRAME]);
        }

#if ONE_STOP_BIT_FIX
        else if( bit_num == STOP_BIT_NUM
                 && bit_size >= FRAMESIZE
               )
        {
            /* Shorten the stop bit to 1 bit length if it will be done this frame */
            DEBUG(1,
                fprintf(stdout,"tty_gen(): Falling behind: stop_bit == 1.0: bit_num = %d bit_size = %3d FRAMESIZE = %3d\n",
                    bit_num,bit_size,FRAMESIZE);
            )
            DEBUG(2,
                fprintf(stdout,"    Stop bits generated  = %3d\n",stop_bit_len-bit_size);
                fprintf(stdout,"    Current data_bit_len = %3d\n",MIN_DATA_BIT_LEN);
                fprintf(stdout,"    Current stop_bit_len = %3d\n",stop_bit_len);
                fprintf(stdout,"    Finish generating 1 stop bit: bit_size before = %d ",bit_size);
            )

            bit_size = bit_size + MIN_DATA_BIT_LEN - stop_bit_len;

            DEBUG(2,fprintf(stdout,"after = %d\n",bit_size);)
         }
#endif /* ONE_STOP_BIT_FIX */

    } /* end if( falling behind) */


    p_buf = outbuf;                                         OP_COUNT(MEM_MOVE);
    num = length;                                           OP_COUNT(MEM_MOVE);
                                                            OP_COUNT(CONDITIONAL+SUB);
    if( bit_size-length < 0 )
    {
        num = bit_size;                                     OP_COUNT(MEM_MOVE);
    }

    write_flag = 0;                                         OP_COUNT(MEM_INIT);


    while( num > 0 )
    {

/*-----------------------------------------------------------------------*/
DEBUG(3,
        fprintf(stdout,"(%d,%2d): (%d,%2d) bit_num = %d  bit_size = %3d  length = %3d num = %d\n",
            counter_hist[CURRENT_FRAME], char_hist[CURRENT_FRAME],
            current_counter,(current_char>>1)&DATA_BIT_MASK, bit_num, bit_size, length, num);
)
/*-----------------------------------------------------------------------*/
                                                            OP_COUNT(CONDITIONAL+SUB);
        /* If current character non_tty, get out of loop */
        if( current_counter == NON_TTY )
        {
              init_tty_gen(NON_TTY,0);
              break;
        }

        /* else if silence, generate silence */
        else if( current_counter == TTY_SILENCE )
        {
                                                            OP_COUNT(CONDITIONAL+SUB);
            for( i=0 ; i < num ; i++ )
            {
                p_buf[i] = 0;                               OP_COUNT(ARRAY_INIT);
            }
            init_tty_gen(TTY_SILENCE,TTY_SILENCE_CHAR);         /* init in preparation for next char */
            write_flag = 1;                                 OP_COUNT(MEM_INIT);
        }
        else
        {
                                                            OP_COUNT(CONDITIONAL+SUB);
            bit = current_char & (1 << bit_num);            OP_COUNT(AND+SHIFT);
                                                            OP_COUNT(CONDITIONAL);
            if( bit == 0 )
            {
                bit = SPACE_FREQ;                           OP_COUNT(MEM_INIT);
            }
            else
            {
                bit = MARK_FREQ;                            OP_COUNT(MEM_INIT);
            }
                                                            OP_COUNT(CONDITIONAL+SUB);
            if( bit != prev_bit )
            {
                init_tone_gen(tone_param,bit);
            }

            tone_gen(p_buf,bit,BAUDOT_GAIN,num,tone_param);
            write_flag = 1;                                 OP_COUNT(MEM_INIT);

            /* Update for next iteration */

            p_buf += num;                                   OP_COUNT(ADD);
            bit_size -= num;                                OP_COUNT(ADD);
                                                            OP_COUNT(CONDITIONAL+SUB);
            if( bit_size < 1 )                 /* if end of bit */
            {
                bit_num++;                                  OP_COUNT(ADD);

                /*
                *  If a new character immediately follows the
                *  current character, eliminate the mark hold tone.
                */
                                                            OP_COUNT(CONDITIONAL+4*SUB);
                if( bit_num == MARK_HOLD_BIT_NUM )
                {
                    /* If a new character is coming, skip the mark hold tone */

                    if( current_counter != counter_hist[CURRENT_FRAME]
                        && counter_hist[CURRENT_FRAME] >= TTY_COUNTER_START
                        && counter_hist[CURRENT_FRAME] <= TTY_COUNTER_STOP
                      )
                    {
                        bit_num++;                              OP_COUNT(ADD);
                    }
                    else
                    {
                        /* Remove current_counter from history */

                        for( i=CURRENT_FRAME-1 ; i >= 0 ; i-- )
                        {
                                                            OP_COUNT(CONDITIONAL+SUB);
                            if( current_counter == counter_hist[i] )
                            {
                                /* Replace with TTY_FER so that a vote will be taken */
                                counter_hist[i] = TTY_FER;      OP_COUNT(ARRAY_INIT);
                                char_hist[i] = 0;               OP_COUNT(ARRAY_INIT);
                            }
                            else
                            {
                                break;
                            }                                
                        }

                    }
                }

                                                            OP_COUNT(CONDITIONAL);
                if( bit_num > MARK_HOLD_BIT_NUM )               /* if end of character */
                {
                                                            OP_COUNT(CONDITIONAL+SUB);
                    /* if start of silence */
                    if( current_counter == counter_hist[CURRENT_FRAME]
                        || counter_hist[CURRENT_FRAME] == NON_TTY )
                    {
                        counter_hist[CURRENT_FRAME] = TTY_SILENCE;      OP_COUNT(ARRAY_INIT);
                        char_hist[CURRENT_FRAME] = TTY_SILENCE_CHAR;    OP_COUNT(ARRAY_INIT);

                        init_tty_gen(TTY_SILENCE,TTY_SILENCE_CHAR);
                    }

                    /* else... next character begins right away. */
                    else                        /* else start next character */
                    {
                        init_tty_gen( counter_hist[CURRENT_FRAME],
                                      char_hist[CURRENT_FRAME]);
                    }
                }
                else                            /* start of new bit */
                {
                                                            OP_COUNT(CONDITIONAL+SUB);
                    if( bit_num == STOP_BIT_NUM )
                    {
#if ONE_STOP_BIT_FIX
                        /*
                        *  Check if a new character is coming before
                        *  the stop bit is generated.
                        */
                        if( current_counter != counter_hist[CURRENT_FRAME]
                            && counter_hist[CURRENT_FRAME] >= TTY_COUNTER_START
                            && counter_hist[CURRENT_FRAME] <= TTY_COUNTER_STOP
                          )
                        
                        {
                            /* If falling behind, use only 1 stop bit */
                            DEBUG(1,
                                fprintf(stdout,"tty_gen(): Falling behind at start of stop bit: Shortening to 1 stop bit\n");
                            )
                            stop_bit_len = MIN_DATA_BIT_LEN;    OP_COUNT(ADD);
                        }
                        else
                        {
                            /* Otherwise, use 1.5 stop bits */
                            stop_bit_len = DEFAULT_STOP_BIT_LEN;   OP_COUNT(ADD);
                        }
#endif /* ONE_STOP_BIT_FIX */

                        bit_size = stop_bit_len;            OP_COUNT(MEM_MOVE);

                    }
                    else if( bit_num == MARK_HOLD_BIT_NUM )
                    {
                                                            OP_COUNT(CONDITIONAL);
                        bit_size = MARK_HOLD_LEN;           OP_COUNT(MEM_INIT);
                    }
                    else
                    {
                                                            OP_COUNT(CONDITIONAL);
                        bit_size = MIN_DATA_BIT_LEN;        OP_COUNT(MEM_INIT);
                    }
                }
            }
        } /* end if */


        /* Update for next iteration */
        prev_bit = bit;                                     OP_COUNT(MEM_MOVE);
        length = length - num;                              OP_COUNT(SUB);
        num = length;                                       OP_COUNT(MEM_MOVE);
                                                            OP_COUNT(CONDITIONAL);
        if( bit_size-length < 0 )
        {
            num = bit_size;                                 OP_COUNT(MEM_MOVE);
        }

    } /* end while */


    /* Update history buffers for next iteration at the end of the frame */
                                                            OP_COUNT(CONDITIONAL+SUB);
    if( subframe == num_subfr-1 )
    {
        /*--------------------------------------------------------------
        * Sanity check to make sure decoder is in sync with encoder    
        * Make sure the decoder transitions to the next character in   
        * the same frame as the encoder                                 
        * -------------------------------------------------------------*/

DEBUG(1,
        if( current_counter != counter_hist[CURRENT_FRAME]
            && current_counter != TTY_FER
            && current_counter != TTY_SILENCE
            && current_counter != NON_TTY
            && counter_hist[CURRENT_FRAME] != TTY_FER
            && counter_hist[CURRENT_FRAME] != TTY_SILENCE
            && counter_hist[CURRENT_FRAME] != NON_TTY
            && subframe == num_subfr-1
        )
        {
            fprintf(stdout,"tty_gen(): New character came early\n");
            fprintf(stdout,"    current_char = (%d,%d)\n",current_counter,(current_char>>1)&DATA_BIT_MASK);
            fprintf(stdout,"    new_char     = (%d,%d)\n",counter_hist[CURRENT_FRAME],char_hist[CURRENT_FRAME]);
            fprintf(stdout,"    bit_num      = %d\n",bit_num);
            fprintf(stdout,"    bit_size     = %d\n",bit_size);
        }
)


        /* Put last generated character in history buffers */
        counter_hist[CURRENT_FRAME] = current_counter;      OP_COUNT(ARRAY_STORE);
        char_hist[CURRENT_FRAME] = (current_char>>1)&DATA_BIT_MASK;     OP_COUNT(ARRAY_STORE);

        for( i=TTY_BUF_SIZE-1 ; i > 0 ; i-- )
        {
            counter_hist[i] = counter_hist[i-1];            OP_COUNT(ARRAY_STORE);
            char_hist[i] = char_hist[i-1];                  OP_COUNT(ARRAY_STORE);
        }
#if 1
        /* During the mark hold, allow the next character to be generated */
        if( bit_num == MARK_HOLD_BIT_NUM )
        {
            for( i=CURRENT_FRAME ; i >= 0 ; i-- )
            {
                                                            OP_COUNT(CONDITIONAL+SUB);
                /* Remove current_counter from history */
                if( current_counter == counter_hist[i] )
                {                                                            
                    counter_hist[i] = TTY_FER;      OP_COUNT(ARRAY_INIT);
                    char_hist[i] = 0;    OP_COUNT(ARRAY_INIT);
                }
                else
                {
                    break;
                }                                
            }
        }
#endif

    }

    return(write_flag);
        
} /* end tty_gen() */

