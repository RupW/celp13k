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
#define DEBUG(x)
#endif

#define TTY_SILENCE_THRESH      100
#define TTY_POWER_THRESH        0.2

static short    counter_buf[TTY_BUF_SIZE];
static short    char_buf[TTY_BUF_SIZE];
static short    rxtx_state;
extern short    stop_bit_len;

/***********************************************************************
*   init_tty_rxtx()
************************************************************************/
void init_tty_rxtx()
{
    short   i;

    init_baudot_to_dit();
    init_dit_to_ascii();
    init_tty_gen(NON_TTY,0);
    stop_bit_len = STOP_BIT_LEN;

    for( i=0 ; i < TTY_BUF_SIZE ; i++ )
    {
        counter_buf[i] = TTY_SILENCE;
        char_buf[i] = TTY_SILENCE_CHAR;
    }

    rxtx_state = NON_TTY_MODE;
}



/***********************************************************************
*   tty_rxtx()
************************************************************************/
short tty_rxtx(
    short   outbuf[],       /* (o): output pcm buffer       */
    short   inbuf[],        /* (i): input pcm               */
    short   len,            /* (i): length of buffers       */
    short   fer_flag        /* (i): frame erasure flag      */
)
{

    short   i;
    short   dit_buf[DITS_PER_FRAME];
    short   tty_char;
    short   counter;



    baudot_to_dit( dit_buf,
                   inbuf,
                   TTY_POWER_THRESH,
                   TTY_SILENCE_THRESH,
                   fer_flag );


    /************* DEBUG *************************/
    for( i=0 ; i < DITS_PER_FRAME ; i++ )
    {
        if( dit_buf[i] == UNKNOWN )
            dump_double_value( 4092.0,16,dit_fp);
        else if( dit_buf[i] == ERASED )
            dump_double_value( -4092.0,16,dit_fp);
        else
            dump_double_value( 20000.0*dit_buf[i],16,dit_fp);
    }
    /************* END DEBUG *************************/

    /* Restore values from previous frame */
    counter = counter_buf[CURRENT_FRAME];
    tty_char = char_buf[CURRENT_FRAME];

    dit_to_ascii( &tty_char, &counter, dit_buf);

    DEBUG(
        switch(counter)
        {
            case NON_TTY:
                fprintf(stdout,"tty_rxtx(): dit2a output: non_tty\n");
                break;
            case TTY_ONSET:
                fprintf(stdout,"tty_rxtx(): dit2a output: onset\n");
                break;
            case TTY_SILENCE:
                fprintf(stdout,"tty_rxtx(): dit2a output: silence\n");
                break;
            default:
                fprintf(stdout,"tty_rxtx(): dit2a output: (%d,%2d)\n",
                        counter,tty_char);
        }
    )
                
    /* Store for next frame */
    counter_buf[CURRENT_FRAME-1] = counter;
    char_buf[CURRENT_FRAME-1] = tty_char;


    /*
    *  Generate the Baudot signal, but create a buffer to match
    *  the interface required by tty_gen()
    */

    for( i=0 ; i < CURRENT_FRAME-1 ; i++ )
    {
        counter_buf[i] = TTY_SILENCE;
        char_buf[i] = TTY_SILENCE_CHAR;
    }
    counter_buf[CURRENT_FRAME] = counter;
    char_buf[CURRENT_FRAME] = tty_char;

    /*----------------------------------------------------------------*/
    DEBUG(
    fprintf(stdout,"-------------------------------\n");
    for( i=CURRENT_FRAME ; i < TTY_BUF_SIZE ; i++ )
    {
        if( counter_buf[i] == NON_TTY )
            fprintf(stdout," ( non)");
        else if( counter_buf[i] == TTY_FER )
            fprintf(stdout," ( FER)");
        else if( counter_buf[i] == TTY_SILENCE && char_buf[i] == TTY_SILENCE_CHAR)
            fprintf(stdout," silnce");
        else if( counter_buf[i] == TTY_ONSET && char_buf[i] == TTY_ONSET_CHAR)
            fprintf(stdout,"  onset");
        else
            fprintf(stdout," (%d,%2d)",counter_buf[i],char_buf[i]);
    }
    fprintf(stdout,"\n");
    );
    /*----------------------------------------------------------------*/

   rxtx_state = tty_gen( outbuf, len, 1, 1, counter_buf, char_buf );

   return(rxtx_state);

}

            


