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
#include "typedef.h"
#include "tty.h"
#include "tty_dbg.h"

#define DUMP_DIR        ""
#define OPEN            ((FILE *) !NULL)

short   tty_debug_flag          = 0;
short   tty_debug_print_flag    = 0;

FILE    *acbgain_fp     = NULL;
FILE    *dec_acbk_fp    = NULL;
FILE    *dec_fcbk_fp    = NULL;
FILE    *dit_fp         = OPEN;
FILE    *enc_fcbk_fp    = NULL;
FILE    *fcbgain_fp     = NULL;
FILE    *lag_fp         = NULL;
FILE    *pow_fp         = OPEN;
FILE    *pow01_fp       = NULL;
FILE    *rate_fp        = NULL;

FILE    *debugfp        = NULL;         /* debug text file */
FILE    *tempfp         = NULL;
FILE    *temp2fp        = NULL;

/**********************************************************************
* open_dumpfile()
***********************************************************************/

void open_dumpfile( FILE **dumpfp, char *basename )
{
    char        filename[256];

    if( *dumpfp != NULL )
    {
        sprintf(filename,"%s%s",DUMP_DIR,basename);
        fprintf(stderr,"Opening file %s\n",filename);
        if( (*dumpfp=fopen(filename,"wb")) == NULL )
        {
            fprintf(stderr,"debug.c: Error opening %s\n", filename );
            exit(1);
        }
    }
    return;

} /* end open_dumpfile() */

/**********************************************************************
* debug()
***********************************************************************/

void tty_debug()
{

    char        filename[256];

    if( tty_debug_flag == 0 )
    {
        return;
    }

    /* File for dumping adaptive codebook gain */
    open_dumpfile(&acbgain_fp, "acbgain.pcm");

    /* File for dumping decoder's adaptive codebook */
    open_dumpfile(&dec_acbk_fp, "dec_acbk.pcm");

    /* File for dumping decoder's fixed codebook */
    open_dumpfile(&dec_fcbk_fp, "dec_fcbk.pcm");

    /* File for dumping dits */
    open_dumpfile(&dit_fp, "dit.pcm");

    /* File for dumping encoder's fixed codebook */
    open_dumpfile(&enc_fcbk_fp, "enc_fcbk.pcm");

    /* File for dumping fixed codebook gain */
    open_dumpfile(&fcbgain_fp,"fcbgain.pcm");

    /* File for dumping pitch lag */
    open_dumpfile(&lag_fp,"lag.pcm");

    /* File for dumping pitch lag */
    open_dumpfile(&pow_fp,"pow.pcm");

    /* File for dumping pitch lag */
    open_dumpfile(&pow01_fp,"pow01.pcm");

    /* File for dumping rate info */
    open_dumpfile(&rate_fp,"rate.pcm");

    /* File for dumping text */
    if( debugfp != NULL )
    {
        sprintf(filename,"%s\\debug.txt",DUMP_DIR);
        fprintf(stderr,"Opening file %s\n",filename);
        if( (debugfp=fopen(filename,"w")) == NULL )
        {
            fprintf(stderr,"debug.c: Error opening %s\n", filename );
            exit(1);
        }
    }
        
    /* File for dumping rate */
    open_dumpfile(&tempfp,"temp.pcm");

    /* File for dumping rate */
    open_dumpfile(&temp2fp,"temp2.pcm");

} /* end debug() */


/**********************************************************************
* dump_array(): Floating Point Version
***********************************************************************/

void dump_float_array( float buf[], short num, FILE *fp )
{

    short       short_temp;
    int         j;

    if( fp != NULL && tty_debug_flag != 0 )
    {
      for( j=0 ; j < num ; j++ )
      {
        if( buf[j] > 32767.0 )
            short_temp = (short) 32767;
        else if( buf[j] < -32767.0 )
            short_temp = (short) -32767;
        else
            short_temp = (short) buf[j];

        fwrite(&short_temp,sizeof(short),1,fp);
      }
    }


} /* end dump_array */

/**********************************************************************
* dump_array(): Fixed Point Version
***********************************************************************/

void dump_short_array( short buf[], short num, FILE *fp )
{
    if( fp != NULL && tty_debug_flag != 0 )
    {
        fwrite(buf,sizeof(short),num,fp);
    }
} /* end dump_array */


/**********************************************************************
* dump_value(): Floating Point Version
***********************************************************************/

void dump_double_value( double value, short num, FILE *fp )
{
    short       short_temp;
    int         j;

    if( fp != NULL && tty_debug_flag != 0 )
    {
        if( value > 32767.0 )
            short_temp = (short) 32767;
        else if( value < -32767.0 )
            short_temp = (short) -32767;
        else
            short_temp = (short) value;

        for( j=0 ; j < num ; j++ )
            fwrite(&short_temp,sizeof(short),1,fp);
    }

} /* end dump_array */
        
/**********************************************************************
* dump_value(): Fixed Point Version
***********************************************************************/

void dump_short_value( short value, short num, FILE *fp )
{
    int         j;

    if( fp != NULL && fp != OPEN && tty_debug_flag != 0 )
    {
        for( j=0 ; j < num ; j++ )
        {
            fwrite(&value,sizeof(short),1,fp);
        }
    }

} /* end dump_array */

