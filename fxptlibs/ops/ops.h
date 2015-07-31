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

#ifndef _DSP_OPS_H_
#define _DSP_OPS_H_

#include <stdio.h>

/*===========================================================================*/
/*         ..Defines.                                                        */
/*---------------------------------------------------------------------------*/

#define ABS                     1
#define ADD                     1
#define AND                     1
#define ARRAY_INIT              1
#define ARRAY_STORE             1
#define CONDITIONAL             5
#define L_ADD                   1
#define L_MOVE                  1
#define L_MPY_LL                8
#define L_MPY_LS                5
#define L_SUB                   1
#define MAC                     1
#define MAX_VAL                 1
#define MEM_MOVE                2
#define MEM_INIT                1
#define MPY                     1
#define OR                      1
#define SHIFT                   1
#define SUB                     1

/*===========================================================================*/
/*         ..Macros.                                                        */
/*---------------------------------------------------------------------------*/
#define OP_COUNT(x) dsp_op_counter+=(x)
#define OP_RESET    dsp_op_counter=0
#define OP_GET      dsp_op_counter

/*===========================================================================*/
/*         ..Structures.                                                     */
/*---------------------------------------------------------------------------*/
typedef struct {
    unsigned long   op_save;
    unsigned long   total_ops;
    unsigned long   max_ops;
    unsigned long   max_op_frame;
    unsigned long   min_ops;
    unsigned long   min_op_frame;
    unsigned long   n_executions;
    float           avg_ops_per_execution;
} OpStat;

/*===========================================================================*/
/*         ..Globals.                                                        */
/*---------------------------------------------------------------------------*/
extern unsigned long  dsp_op_counter;

/*===========================================================================*/
/*         ..Functions.                                                      */
/*---------------------------------------------------------------------------*/
extern void  zeroOps   (OpStat*);
extern void  resetOps  (OpStat*);
extern void  updateOps (OpStat*);
extern void  fprintOps (FILE*, OpStat*);

#endif
