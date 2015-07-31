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


#ifndef _TTY_H_
#define _TTY_H_

#include <stdio.h>
#include "typedef_fx.h"
#include "tty_hdr.h"

#define TTY_VERSION     "SMV TTY 31 Aug 2001"

#define MIN_INPUT_LEVEL_FIX     1
#define TTY_GEN_50_BAUD_FIX     1


/* Thresholds for get_tty_bit() and get_tty_char() */

#define MIN_BIT_LEN         8   /* min # of dits, good and bad, to make a bit */
#define MAX_BIT_LEN         13  /* max # of dits, good and bad, to make a bit */

#define DITBUF_LEN          MAX_BIT_LEN+DITS_PER_FRAME+1

#define TTY_DISABLED        0
#define TTY_NO_GAIN         1
#define TTY_AUDIO_DET       2

#define TTY_BIT_HIST_LEN    9
#define MEM_BIT_IDX         0
#define START_BIT_IDX       1
#define STOP_BIT_IDX        7

#define START_BIT           0
#define STOP_BIT            1
#define BAUDOT_GAIN         8192
#define START_BIT_LEN       176
#define DATA_BIT_LEN        176
#define STOP_BIT_LEN        (2*DATA_BIT_LEN)
#define TTY_CHAR_LEN        (START_BIT_LEN+5*DATA_BIT_LEN+STOP_BIT_LEN)

#define NUM_TTY_RATES           2
#define TTY_45_BAUD             0
#define TTY_50_BAUD             1
#define DEFAULT_TTY_BAUD_RATE   TTY_45_BAUD

#define BIT_LEN_45_BAUD         11      /* # dits in a 50 Baud TTY bit */
#if TTY_GEN_50_BAUD_FIX
#define DATA_BIT_LEN_45_BAUD    176
#else
#define DATA_BIT_LEN_45_BAUD    172
#endif
#define STOP_BIT_LEN_45_BAUD    ((3*DATA_BIT_LEN_45_BAUD) >> 1 )

#define BIT_LEN_50_BAUD         10      /* # dits in a 50 Baud TTY bit */
#if TTY_GEN_50_BAUD_FIX
#define DATA_BIT_LEN_50_BAUD    160
#define STOP_BIT_LEN_50_BAUD    280 /* 1.75 bits for stop bit */
#else
#define DATA_BIT_LEN_50_BAUD    156
#define STOP_BIT_LEN_50_BAUD    ((3*DATA_BIT_LEN_50_BAUD) >> 1 )
#endif

#define MARK_HOLD_BIT       1
#define MARK_HOLD_LEN       2400    /* 300 ms. */

#define DEFAULT_DATA_BIT_LEN    DATA_BIT_LEN_45_BAUD
#define DEFAULT_STOP_BIT_LEN    STOP_BIT_LEN_45_BAUD



#define MARK_FREQ               0x3A1C  /* Q14 2*cos(w) 1400 Hz */
#define SPACE_FREQ              0x1406  /* Q14 2*cos(w) 1800 Hz */

/* Define dit values */
#define LOGIC_0         0
#define LOGIC_1         1
#define SILENCE         2
#define ERASE           3
#define UNKNOWN         4 /*NON_TTY*/
#define START_BIT_VAL   LOGIC_0
#define STOP_BIT_VAL    LOGIC_1


/* Define values that can be used in char_counter and tty_char */
#define TTY_CHAR_MIN            0
#define TTY_CHAR_MAX            31

/* TTY states */
#define NON_TTY_MODE            1
#define TTY_MODE_NOT_FRAMED     2
#define TTY_MODE_FRAMED         4

#ifndef PI
#define PI              3.14159265358979323846
#endif

#ifndef MAX
#define MAX(x,y)    (((x)>(y))?(x):(y))
#endif

#ifndef MIN
#define MIN(x,y)    (((x)<(y))?(x):(y))
#endif

/*** TTY Globals ***/
extern Word16   tty_option;
extern Word16   tty_enc_flag;
extern Word16   tty_enc_header;
extern Word16   tty_enc_char;
extern Word16   tty_enc_baud_rate;
extern Word16   tty_dec_flag;
extern Word16   tty_dec_header;
extern Word16   tty_dec_char;
extern Word16   tty_dec_baud_rate;
extern Word16   data_flag;
extern Word16   speech_data_flag;

extern void init_tty_enc(
    Word16   *tty_char,
    Word16   *counter,
    Word16   *tty_baud_rate
);

extern Word16 tty_enc(
    Word16   *tty_char,      /* (o): tty character           */
    Word16   *counter,       /* (o): tty character counter   */
    Word16   *tty_baud_rate, /* (i/o): prev/new character counter    */
    float    pcm_buf[],      /* (i): input pcm               */
    Word16   len             /* (i): length of pcm buffer (FRAMESIZE)    */
);

extern void init_dit_to_ascii();

extern void dit_to_ascii(
    Word16   *tty_char,      /* (i/o): prev/new character decision   */
    Word16   *char_counter,  /* (i/o): prev/new character counter    */
    Word16   *tty_baud_rate, /* (i/o): prev/new character counter    */
    Word16   ditInbuf[]      /* (i): dits for current frame          */
);

extern void get_tty_state(
    Word16   *ttyState,              /* (i/o): TTY State                 */
    Word16   first_tty_char_flag,    /* (i): first char detected flag    */
    Word16   tty_bit_hist[],         /* (i): TTY bit history buffer        */
    Word16   tty_bit_len_hist[]      /* (i): TTY bit length history buffer */
);

extern void init_get_tty_bit();

extern Word16 get_tty_bit(
    Word16   tty_bit_hist[],         /* (i/o): bit decision          */
    Word16   tty_bit_len_hist[],     /* (i/o): bit length            */
    Word16   *bit_index,             /* (o): bit index in ditbuf     */
    Word16   *inbuf,                 /* (i/o): dit buffer            */
    Word16   baud_rate               /* (i): baud_rate               */
);

extern Word16 get_tty_char(
    Word16   *tty_char,              /* (o): TTY character               */
    Word16   tty_bit_hist[],         /* (i): TTY bit history buffer      */
    Word16   tty_bit_len_hist[]      /* (i): TTY bit length history buffer */
);

extern void init_baudot_to_dit();

extern void baudot_to_dit(
    Word16    ditbuf[],              /* (o): Dit decisions           */
    Word16    inbuf[]                /* (i): Input PCM               */
);

extern void init_tty_dec();

extern Word16 tty_dec(
    Word16   buf[],
    Word16   acb_gain,
    Word16   counter,
    Word16   tty_char,
    Word16   tty_baud_rate,
    Word16   fer_flag,
    Word16   subframe,
    Word16   num_subfr,
    Word16   length
);

extern void init_tty_gen(
    Word16 counter,
    Word16 tty_char,
    Word16 tty_baud_rate
);

extern Word16 tty_gen(
    Word16   outbuf[],
    Word16   length,
    Word16   subframe,
    Word16   num_subfr,
    Word16   counter_hist[],
    Word16   char_hist[],
    Word16   tty_rate_hist[]
);

extern void tone_gen(
    Word16       outbuf[],
    Word16       freq,
    Word16       volume,
    Word16       len,           
    Word16       param[]);

extern void init_tone_gen(
    Word16       param[],        /* (o) tone generation parameters       */
    Word16       freq );         /* (i) Desired freq in Hz               */

extern void tty_rate(
    Word16   *tty_baud_rate,     /* (i/o): prev/new tty baud rate    */
    Word16   tty_bit_len_hist[]  /* (i): bit len buffer              */
);

extern void tty_header_in(Word16 *counter);
extern void tty_header_out(Word16 *counter);

#endif /* _TTY_H_ */
