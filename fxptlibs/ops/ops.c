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

/*===========================================================================*/
/*         ..Includes.                                                       */
/*---------------------------------------------------------------------------*/
#include  <stdio.h>
#include  "ops.h"

/*===========================================================================*/
/*         ..Globals.                                                        */
/*---------------------------------------------------------------------------*/
unsigned long  dsp_op_counter = 0;

/*===========================================================================*/
/*         ..Functions.                                                      */
/*---------------------------------------------------------------------------*/
void  zeroOps (
	OpStat*  op
)
{
	/*....execute....*/
	op->op_save      = 0;
	op->total_ops    = 0;

	op->max_ops      = 0;
	op->max_op_frame = 0;
    op->min_ops      = 0;
	op->min_op_frame = 0;

	op->n_executions = 0;
	op->avg_ops_per_execution = 0.0;
}

/*===========================================================================*/
/*         ..Functions.                                                      */
/*---------------------------------------------------------------------------*/
void  resetOps (
	OpStat*  op
)
{
	/*....execute....*/
	op->op_save      = dsp_op_counter;
}

/*===========================================================================*/
/*         ..Functions.                                                      */
/*---------------------------------------------------------------------------*/
void  updateOps (
	OpStat*  op
)
{
	/*....(local) variables....*/
    unsigned long  cur_ops;

	/*....execute....*/
	cur_ops = dsp_op_counter - op->op_save;
    if (cur_ops > op->max_ops)
        op->max_ops = cur_ops;

    if (cur_ops < op->min_ops || op->min_ops == 0 )
        op->min_ops = cur_ops;

	op->n_executions++;
	op->total_ops += cur_ops;
	op->avg_ops_per_execution = (float)(op->total_ops) / (float)(op->n_executions);
}

/*===========================================================================*/
/*         ..Functions.                                                      */
/*---------------------------------------------------------------------------*/
void  fprintOps (
    FILE*    fp,
	OpStat*  op
)
{
	/*....execute....*/
        fprintf(fp,"Ops Report:\n");
        fprintf(fp,"  Max Ops/Call = %lu\n", op->max_ops );
        fprintf(fp,"  Min Ops/Call = %lu\n", op->min_ops );
        fprintf(fp,"  Avg Ops/Call = %g\n",  op->avg_ops_per_execution );
        fprintf(fp,"  Num Calls    = %lu\n", op->n_executions );

        return;
}

