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
/* defines.h - basic defines for celp coder   */


#define EPS        1E-08

#ifndef PI
#define PI         3.1415927
#endif

#define PF_OFF     0
#define PF_ON      1
#define PITCH      0
#define CB         1
#define ENCODER    0
#define DECODER    1
#define NOT_WGHTED 0
#define WGHTED     1
#define NO         0
#define YES        1
#define NEGATIVE   1
#define POSITIVE   0
#define BOTH       0
#define ONLY_UNWGHTED 1
#define UNLIMITED  -1

/* celp13k's own padded packet format */
#define FORMAT_PACKET 0
/* QCP, RFC 3625 */
#define FORMAT_QCP 1

/* signed 16-bit host-endian 8 KHz mono audio */
#define FORMAT_RAW_AUDIO 0
/* encoder state output for debugging - treat as raw */
#define FORMAT_DEBUG_OUTPUT 0

#define PRED 0
#define VQT  1

/* Begin pack */
#define LSPs    0  /* Eighth Rate Only */
#define PGAIN   1  /* Pitch Gain */
#define PLAG    2  /* Pitch Lag */

#define LSPVs   4  /* LSP Vectors Non-Eighth Rate  */
#define PFRAC   5  /* Bit for Fractional Pitch  */

#define CBGAIN  6  /* Codebook Gain */
#define CBINDEX 7  /* Codebook Index */
#define CBSIGN  8  /* Codebook Sign for Gain?? */
#define CBSEED  9  /* Codebook Seed */
#define RESERVE 10 /* Reserved Bit */
/* End pack */

#define QUANT03BIT     8
#define QUANT10BIT  1024
#define QUANT12BIT  4096
#define QUANT13BIT  8192
#define QUANT14BIT 16384
#define QUANT15BIT 32768
#define QUANT16BIT 65536
