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
/* celp13k.c - Basic Framework Code for Speech Coder Work	*/

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <memory.h>
#include <string.h>
#include <sys/types.h>
#ifndef _MSC_VER
#include <sys/file.h>
#endif
#include <sys/stat.h>
#include <math.h>
#ifndef __SUNOS__
#include <getopt.h>
#endif

#include "celp.h"
#include "tty.h"
#include "tty_dbg.h"
#include "ops.h"

/* Version 2.1 04 Dec 2000:
*    Update TTY with false alarm fixes that require TTY_SILENCE to
*    be received in a minimum of 3 frames and change in the encoder
*    to guarantee that a minimum of 4 TTY_SILENCE messages are sent.
*/

char *prog;
char prog_desc[] = "\n\nCELP 13K coder - \nRevision Date: 03 Mar 2004\n";
char *rates_str[NUMMODES] = {"BLANK", "EIGHTH", "QUARTER", "HALF", "FULL"};

extern char     *trans_fname;
extern int      fer_sim_seed;

extern void   fer_sim(int *rate);

void usage( struct CONTROL *control )
{
printf("\n%s: %s\n",prog, prog_desc);
  printf("Usage : %s -i infn -o outfn [-c] [-p] [-f frames] [-k] [-e] \n\t\t [-r] [-l minrate] [-h maxrate]\n",prog);
  printf("\t-i\tinfn\t: input speech file\n");
  printf("\t-o\toutfn\t: output speech file\n");
  printf("\t-e\t\t: output from encoder rather than decoder\n");
  printf("\t-E\t\t: encode only, output (after -o ) will be a packet file\n");
  printf("\t-D\t\t: decode only, input must be a packet file\n");
  printf("\t-r\t\t: output formant residual, rather than speech\n");
  printf("\t-t\t\t: output target after cdbk search, rather than speech\n");
  printf("\t-c\t\t: output codebook output, rather than speech\n");
  printf("\t-p\t\t: output pitch filter output, rather than speech\n");
  printf("\t \t \t:   pitch prefilter and agc are suppressed\n");
  printf("\t-f\tframes\t: stop after `frames' frames have been processed\n");
  printf("\tRATES: 1 - eighth-rate, 2 - quarter-rate, 3 - half-rate, 4 - full-rate,\n");
  printf("\t-l\tminrate\t: minimum rate (see key in above line)\n");
  printf("\t-h\tmaxrate\t: maximum rate (see key two lines above)\n");
  printf("\t-k\t\t: print packet info to stdout\n");
  printf("\t-s\tflag\t: postfilter enable/disable: %d to enable, %d to disable\n",YES, NO);
  printf("\t\t\t  current setting at break time is %d\n",control->pf_flag);
  printf("\t-m\ti\t: reduced rate flag (0 <= i <= 4, default is 0)\n");
  printf("\t-z\t\t: turn pre_filter and agc off (default is on)\n");
  printf("\tTTY OPTIONS:\n");
  printf("\t-T\t\t: TTY Processing (0=OFF 1=NO_GAIN) (default=NO_GAIN)\n");
  printf("\t-d\t\t: TTY debugging files (0=OFF 1=ON) (default is off)\n");
  printf("\t-M\t\t: FER Simulator Markov Transition Prob. File\n");
  printf("\t-S\t\t: FER Simulator initial seed (default=random (0))\n");
  exit(-1);
}

void print_welcome_message()
{
printf("\
\n\
===============================================\n\
       CDMA 13K Speech Coder TTY EDITION\n\
               Ballot Version 1.0\n\
               July 12, 1999\n\
===============================================\n\n\
");
        
}

void print_farewell_message()
{
    printf("Done...\n");
}


/*======================================================================*/
/*         ..Returns number of frames in a binary file.                 */
/*----------------------------------------------------------------------*/
unsigned long GetNumFrames (
	FILE*  fp,
        int    blocksize
)
{
	/*....(local) variables....*/
        unsigned long position;
        unsigned long numFrames;

	/*....execute....*/
	position = ftell(fp);
	fseek(fp,0L,2);
        numFrames = (ftell(fp)+blocksize-1) / blocksize;
	fseek(fp,position,0);
	return(numFrames);
}

char prog_opts[] = "abcd:DEeF:f:h:i:kl:M:m:o:pPrS:s:T:tuvwz"; /* command line flags */
extern char *optarg;
extern int  optind;

void parse_command_line(
     int             argc,
     char            *argv[],
     char            *fn_inspeech,
     char            *fn_outspeech,
     struct CONTROL  *control 
)
{
  int i,err;
  prog=argv[0];

  control->min_rate  =EIGHTH;
  control->max_rate  = FULLRATE_VOICED;
  control->avg_rate  =9.0;
  control->target_snr_thr  =10.0;
  control->pf_flag   =PF_ON;
  control->cb_out    =NO;
  control->pitch_out =NO;
  control->num_frames=UNLIMITED;
  control->print_packets=NO;
  control->output_encoder_speech=NO;
  control->form_res_out=NO;
  control->reduced_rate_flag=NO;
  control->unvoiced_off=NO;
  control->pitch_post   =YES;
  control->per_wght = PERCEPT_WGHT_FACTOR;
  control->target_after_out=NO;
  control->decode_only=NO;
  control->encode_only=NO;
  control->fractional_pitch=YES;

  /* TTY OPTIONS */
  fer_sim_seed = 0;
  trans_fname = NULL;
  tty_option = TTY_NO_GAIN;

  err=0;
  while ((i = getopt(argc, argv, prog_opts)) != EOF)
  {
  switch (i) {
  case 'u':
    control->unvoiced_off=YES;
    printf("unvoiced mode is turned off (only meaningful in RRM)\n");
    break;
  case 'z':
    control->pitch_post=NO;
    printf("pitch pre-filter is turned off\n");
    break;
  case 'w':
    control->per_wght=1.0;
    printf("perceptual weighting off, compare enc & dec\n");
    break;
  case 'r':
    control->output_encoder_speech=YES;
    control->form_res_out=YES;
    printf("Output file will contain the formant residual\n");
    break;
  case 't':
    control->output_encoder_speech=YES;
    control->target_after_out=YES;
    printf("Output file will contain target after codebook search\n");
    break;
  case 'c':
    control->cb_out=YES;
    printf("Output file will contain output of codebook\n");
    break;
  case 'D':
    control->decode_only=YES;
    printf("Decoding packet files\n");
    break;
  case 'p':
    control->pitch_out=YES;
    printf("Output file will contain output of pitch filter\n");
    break;
  case 'k':
    control->print_packets=YES;
    printf("Packet info will be printed to stdout.\n");
    break;
  case 'e':
    control->output_encoder_speech=YES;
    printf("Output file will contain output of encoder's decoder\n");
    break;
  case 'E':
    control->encode_only=YES;
    printf("Encoding to packet files\n");
    break;
  case 'f':
    sscanf(argv[optind-1],"%d",&(control->num_frames));
    printf("Will only process up to %d frames\n",control->num_frames);
    break;
  case 'F':
    sscanf(argv[optind-1],"%d",&(control->fractional_pitch));
    if(control->fractional_pitch)
      printf("Fractional Pitch Enabled\n");
    else
      printf("No Fractional Pitch\n");
    break;
  case 'l':
    sscanf(argv[optind-1],"%d",&(control->min_rate));
    printf("Lowest Rate set to %s rate\n", rates_str[control->min_rate]);
    break;
  case 'h':
    sscanf(argv[optind-1],"%d",&(control->max_rate));
    printf("Highest Rate set to %s rate\n", rates_str[control->max_rate]);
    break;
  case 'm':
    sscanf(argv[optind-1],"%d",&(control->reduced_rate_flag));
    if(control->reduced_rate_flag == 3){
      control->avg_rate  =14.4*(8.-control->reduced_rate_flag)/8.0; 
      control->target_snr_thr  =10.0;
    }
    else if(control->reduced_rate_flag == 4){
      control->avg_rate  =14.4*(8.-control->reduced_rate_flag)/8.0;
      control->target_snr_thr  = 7.0;
    }
    if(control->reduced_rate_flag > 4 || control->reduced_rate_flag < 0){
      printf("Parameters in RRM are out of bounds\n");
      printf("Setting reduced_rate_flag to zero\n");
      control->reduced_rate_flag = 0;
    }
    else if(control->reduced_rate_flag == 3){
    printf("Operating in reduced rate mode=%d: %f kbps for active speech\
 (channel rate) \n", 
	     control->reduced_rate_flag, control->avg_rate);
  }
    else
      printf("Operating in reduced rate mode=%d\n",
	     control->reduced_rate_flag);
    break;
  case 's':
    sscanf(argv[optind-1],"%d",&(control->pf_flag));
    if (control->pf_flag==YES) {
      printf("Postfilter enabled\n");
    }
    else if (control->pf_flag==NO) {
      printf("Postfilter disabled\n");
    }
    else {
      printf("Unknown postfilter flag\n");
      usage(control);
    }
    break;
  case 'i':
    err++;
    sprintf(fn_inspeech,"%s",argv[optind-1]);
    break;
  case 'o':
    err++;
    sprintf(fn_outspeech,"%s",argv[optind-1]);
    break;
  case 'd':
    tty_debug_flag = atoi(argv[optind-1]) & 2;
    tty_debug_print_flag = atoi(argv[optind-1]) & 1;
    break;
  case 'T':
    tty_option = atoi(argv[optind-1]);
    break;
  case 'M':
    trans_fname = argv[optind-1];
    break;
  case 'S':
    fer_sim_seed = atoi(argv[optind-1]);
    break;
  default:
    usage(control);
  }
  } /* end while */

  if (err<2) {
    printf("%s: Some required command line arguments missing\n", prog);
    for( i=0 ; i < argc ; i++ )
    {
        fprintf(stderr,"%s ",argv[i]);
    }
    fprintf(stderr,"\n");
    usage(control);
  }

  if(control->decode_only==YES &&   control->encode_only==YES){
    fprintf(stderr,"parse_command_line: decode_only and encode_only can't both be YES\n");
    exit(-2);
  }

  if(control->min_rate > control->max_rate && control->min_rate != QUARTERRATE_UNVOICED ){
    fprintf(stderr,
	    "parse_command_line: Illegal min_rate (%d) and max_rate (%d)\n",
	    control->min_rate, control->max_rate);
    exit(-2);
  }
  if(control->min_rate == QUARTERRATE_VOICED || 
                           control->max_rate ==QUARTERRATE_VOICED ){

    fprintf(stderr,
  "parse_command_line: min_rate (%d) or max_rate (%d) is currently illegal\n", 
	    control->min_rate, control->max_rate);
    exit(-2);
  }

}/* end of parse_command_line() */


int main( int argc, char *argv[] )
{
  int   i;
  char  fn_inspeech[80], fn_outspeech[80];
  FILE  *fin, *fout;
  int   numread, nsamp;
  float temp[MAXSF];
  unsigned long frame_num;
  unsigned long total_frames;
  unsigned long fer_count;
  struct ENCODER_MEM     encoder_memory;
  struct DECODER_MEM     decoder_memory;
  struct PACKET          packet;
  struct CONTROL         control;

#if USE_CALLOC
  float *in_speech;
  float *out_speech;
#else
    float   in_speech[FSIZE+LPCSIZE-FSIZE+LPCOFFSET];
    float   out_speech[FSIZE];
#endif

  memset(&encoder_memory, 0, sizeof(encoder_memory));
  memset(&decoder_memory, 0, sizeof(decoder_memory));
  memset(&packet, 0, sizeof(packet));
  memset(&control, 0, sizeof(control));


#if USE_CALLOC
  alloc_mem_for_speech(&in_speech, &out_speech);
#endif

  for(i = 0; i < argc; i++)
    fprintf(stdout, "%s ", argv[i]);
  fprintf(stdout, "\n");

  parse_command_line(argc, argv, fn_inspeech, fn_outspeech, &control);

  initialize_encoder_and_decoder(&encoder_memory, &decoder_memory, 
				 &control);

  print_welcome_message();

  /*** Init TTY/TDD Routines and Varibles ***/

  if( tty_debug_flag )
      tty_debug();

  if( tty_option == TTY_NO_GAIN )
  {
      fprintf(stdout," TTY OPTION = NO GAIN\n");
      init_tty_enc( &tty_enc_char, &tty_enc_header, &tty_enc_baud_rate);
      init_tty_dec();
  }
  else
  {
      tty_option = 0;
      fprintf(stdout," TTY OPTION = OFF\n");
  }

  if( trans_fname != NULL )
  {
      fprintf(stdout,"FER SIMULATOR ON:  seed = %d\n",fer_sim_seed);
  }


  for(i = 0; i < LPCORDER; i++)
    packet.lsp[i] = packet.lpc[i] = 0;

  encoder_memory.frame_num = 0;
  frame_num = 0;
  fer_count = 0;

  open_binary_input_file(&fin, fn_inspeech);
  open_binary_output_file(&fout, fn_outspeech);


  if( control.decode_only == YES )
  {
      total_frames = GetNumFrames(fin,sizeof(short)*WORDS_PER_PACKET);
  }
  else
  {
      total_frames = GetNumFrames(fin,sizeof(short)*FSIZE);
  }

  if(control.decode_only == NO)
  {
#if 0
    if (read_samples(fin, in_speech, LPCSIZE-FSIZE+LPCOFFSET)
	!=LPCSIZE-FSIZE+LPCOFFSET)
    {
      printf("Not even enough samples for 1 frame!\n");
      usage(&control);
    }
#else
    for( i=0 ; i < LPCSIZE-FSIZE+LPCOFFSET ; i++ )
    {
        in_speech[i] = 0;
    }
#endif
  }



  /*-----------------------------------------------
  *                   Main Loop
  *------------------------------------------------*/

  while( control.num_frames == UNLIMITED || frame_num < control.num_frames )
  {
    fprintf(stderr,"Processing %lu of %lu  FER = %.2f%%\r",
        frame_num, total_frames, 100.0*fer_count/(frame_num+1));

    if (control.decode_only==NO)
    {
        nsamp = read_samples(fin, &in_speech[LPCSIZE-FSIZE+LPCOFFSET], FSIZE);
        if( nsamp <= 0 )
        {
            break;
        }
        else if(nsamp < FSIZE)
        {
            for (i=nsamp; i<FSIZE; i++)
            {
                in_speech[LPCSIZE-FSIZE+LPCOFFSET+i]=0;
            }
        }

        encoder(in_speech, &packet, &control,
                &encoder_memory, out_speech);

        update_snr(ENCODER, in_speech, out_speech, &(control.snr));

    }

    if (control.decode_only==YES)
    {
        numread = read_packet(fin, packet.data, WORDS_PER_PACKET); 
        if( numread != WORDS_PER_PACKET)
        {
            if(numread != 0)
            {
              fprintf(stderr,
                  "%s: Wrong number of words read: %d\n", argv[0], numread);
            }
            break;
        }
    }

    if(control.encode_only==NO)
    {

        if (control.output_encoder_speech==NO)
        {
            decoder(out_speech, &packet, &control, &decoder_memory);

            if( packet.data[0] == ERASURE )
            {
                fer_count++;
            }

            if(control.decode_only==NO)
            {
                update_snr(DECODER, in_speech, out_speech, &(control.snr));
            }
        }
        i = write_samples(fout,out_speech,FSIZE); 
    }
    else
    {
        if( trans_fname != NULL )
        {
            fer_sim( &(packet.data[0]) );
        }
        if( packet.data[0] == ERASURE )
        {
            fer_count++;
        }
        i = write_packet(fout, packet.data, WORDS_PER_PACKET);
    }


    /***** Update in_speech buffer ***************/

    for (i=0; i<LPCSIZE-FSIZE+LPCOFFSET; i++)
    {
        in_speech[i]=in_speech[i+FSIZE];
    }

    frame_num++;
    encoder_memory.frame_num = frame_num;

  } /* end main while() */


  fclose(fin);
  fclose(fout);

  if ((control.encode_only==NO)&&(control.decode_only==NO))
  {
    compute_snr(&(control.snr), &control);
  }
  if( control.decode_only == NO )
  {
      /* calculate the avg. rate for active speech for the entire file */
      temp[0] = (encoder_memory.full_cnt + encoder_memory.full_force +
	     encoder_memory.full_cnt_t + encoder_memory.full_force_t)*14.4+ 
	    (encoder_memory.half_cnt + encoder_memory.half_force +
	     encoder_memory.half_cnt_t + encoder_memory.half_force_t)*7.2+ 
	    (encoder_memory.quarter_cnt + encoder_memory.quarter_cnt_t)*3.6;
      temp[0] /= (encoder_memory.total_speech_frames+
	      (STATWINDOW-encoder_memory.block_cnt));

      if(control.reduced_rate_flag != 0)
      {
          printf("\n The target_snr_threshold at the end of the run was %f",
             encoder_memory.target_snr_thr);

          printf("\n The average rate for the entire file was %f",temp[0]);
          i = STATWINDOW-encoder_memory.block_cnt+encoder_memory.total_speech_frames;
          temp[1] = i;
          printf("\n The # of speech frames in the file is  = %d",i);

          i = encoder_memory.full_cnt+encoder_memory.full_cnt_t;
          printf("\n The percent of frames at 14.4 is %f",100.0*i/temp[1]);
          i = encoder_memory.full_force+encoder_memory.full_force_t;
          printf("\n The percent of frames forced to  14.4 is %f",100.*i/temp[1]);
          i = encoder_memory.half_cnt+encoder_memory.half_cnt_t;
          printf("\n The percent of frames at 7.2 is %f",100.*i/temp[1]);
          i = encoder_memory.half_force+encoder_memory.half_force_t;
          printf("\n The percent of frames forced to  7.2 is %f",100.*i/temp[1]);
          i = encoder_memory.quarter_cnt+encoder_memory.quarter_cnt_t;
          printf("\n The percent of frames coded at 3.6 is %f\n",100.*i/temp[1]);

      }
  }

  if(control.encode_only == NO)
    print_erasure_count();
  free_encoder_and_decoder(&encoder_memory, &decoder_memory);

#if USE_CALLOC
  free((char*) in_speech);
  free((char*) out_speech);
#endif

  print_farewell_message();

  exit(0);

}/* end of main() */


