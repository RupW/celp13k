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
/* io.c - Basic File I/O for raw binary files                           */

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

#include "coder.h"
#include "tty.h"

/* open_binary_input_file - opens a binary input file		*/
void open_binary_input_file(
    FILE  **fin,
    char *filename
)
{

   if( (*fin=fopen(filename,"rb")) == NULL )
   {
        fprintf(stderr,"ERROR: Unable to open input file %s\n",filename);
        exit(-1);
   }

   printf("Binary input file  - %s\n",filename);
   /*lseek(*fin, (off_t) 0, L_SET);*/
}

/* open_binary_output_file - opens a binary output file		*/
/* 19911017							*/
void open_binary_output_file(
    FILE **fout,
    char *filename
)
{
   printf( "Binary output file - %s\n",filename);
   if( (*fout=fopen(filename,"wb")) == NULL )
   {
        fprintf(stderr,"ERROR: Unable to open output file %s\n",filename);
        exit(-1);
   }
   /*lseek(*fout, (off_t) 0, L_SET);*/
}

/* read_samples - reads "insamples" number of samples from 	*/
/*	binary input file "fin" to float array "inbuf"		*/
/* 	The samples range from -2^15 to 2^15 -1.		*/
/* 	Returns the number of samples read.			*/
int read_samples(
    FILE  *fin,
    float *inbuf,
    int   insamples 
)
{

    int   i;
    int   samplesread;
    short swap;
    short tmpbuf[FSIZE];

    samplesread = fread( tmpbuf+FSIZE-insamples, sizeof(short), insamples, fin);

    /* Zero pad tmpbuf to FSIZE for tty_enc() */
    if( samplesread < FSIZE )
    {
        if( insamples < FSIZE )
        {
            /* If at beginning of file, zero pad beginning of array */
            for( i=0 ; i < FSIZE-samplesread ; i++ )
            {
                tmpbuf[i] = 0;
            }
        }
        else
        {
            /* If at the end of the file, zero pad the end of array */
            for( i=samplesread ; i < FSIZE ; i++ )
            {
                tmpbuf[i] = 0;
            }
        }
    }

    for( i=0 ; i < samplesread ; i++)
    {
        swap = tmpbuf[FSIZE-insamples+i];
        *(inbuf+i)=(float)(swap)/4.0;
    }

    /*free((char*)tmpbuf);*/ /* SAB: removed */
    return(samplesread);
}

/* write_samples - writes "outsamples" number of samples from	*/
/*	float array "outsamples" to binary output file "fout"	*/
/*	Saturates the samples to the allowable range.		*/
/*	Returns the number of samples written.			*/
int write_samples(
    FILE  *fout,
    float *outbuf,
    int   outsamples
)
{

    int   num;
    int   i;
    short swap;
    float ftmp;
    short *tmpbuf;

    tmpbuf= (short *)(calloc((unsigned)outsamples, sizeof(short)));

    for(i=0; i<outsamples; i++ ){
	ftmp=*(outbuf+i) *4.0;
	ftmp=(ftmp<-32768.0)? -32768.0:ftmp;
	ftmp=(ftmp> 32767.0)?  32767.0:ftmp;
	swap=(short)ftmp;
	tmpbuf[i] = swap;
    }
    if( (num=fwrite(tmpbuf, sizeof(short),outsamples,fout))
        != outsamples )
    {
        fprintf(stderr,"Error writing data to output file: num = %d\n",num);
	exit(-1);
    }

    free((char*)tmpbuf);

    return(num);

}/* end of write_samples() */

int read_packet(
    FILE  *fin,
    int   *inbuf,
    int   insamples
)
{
    int   i;
    int   samplesread;
    unsigned short tmp[300];
    unsigned short swap;


    samplesread = fread( tmp, sizeof(short),insamples, fin );

    for (i=0; i < insamples; i++)
    {
      swap = tmp[i];
      inbuf[i] = swap;
    }
    return(samplesread);

}/* end of read_packet() */

int write_packet( 
    FILE  *fout,
    int   *outbuf,
    int   outsamples
)
{
    int   i;
    int   num;
    unsigned short tmp[300];
    unsigned short swap;

    for (i=0; i<outsamples; i++)
    {
      if(outbuf[i] & 0xffff0000)
      {
	fprintf(stderr,
		"write_packet: outbuf[%d] = %d is more than 16 bits\n",
		i, outbuf[i]);
      }
      swap = outbuf[i];
      tmp[i] = swap;
    }
    if( (num=fwrite( tmp, sizeof(short), outsamples, fout))
        != outsamples )
    {
        fprintf(stderr,"Error writing data to output file\n" );
	exit(-1);
    }

    return(num);

}/* end of write_packet() */





