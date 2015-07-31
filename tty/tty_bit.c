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


/*********************************************************************
*
*   get_tty_bit()
*
**********************************************************************/
#include <stdio.h>
#include "typedef.h"
#include "tty.h"
#include "basic_op.h"


#define GOOD_DIT_THRESH         6   /* # of good dits to make a bit */
#define BAD_DIT_THRESH          2  /*2*/
#define TTY_BIT_UNKNOWN_THRESH  5  


/*********************************************************************
*   init_get_tty_bit()
**********************************************************************/
void init_get_tty_bit()
{
    return;

} /* end init_get_tty_bit() */


/*********************************************************************
*   get_tty_bit()
**********************************************************************/
Word16 get_tty_bit(
    Word16   tty_bit_hist[],         /* (i/o): bit decision          */
    Word16   tty_bit_len_hist[],     /* (i/o): bit length            */
    Word16   *bit_index,             /* (o): bit index in ditbuf     */
    Word16   *ditbuf,                 /* (i/o): dit buffer            */
    Word16   baud_rate               /* (i): baud_rate               */
)
{
    Word16 i;
    Word16 j;
    Word16 k;
    Word16 update_flag;
    Word16 bit;
    Word16 good_dits;
    Word16 bad_dits;
    Word16 fail_counter;
    Word16 tty_unknown_counter;
    Word16 len;
    Word16 bit_len;
    Word16 mark_counter;
    Word16 space_counter;




    update_flag = 0;                                
    bit = UNKNOWN;                                  
    bit_len = 0;                                    
    bad_dits = 0;                                   
    tty_unknown_counter = 0;                        
    fail_counter = 0;                               

    for( i = *bit_index ; i < DITBUF_LEN-MIN_BIT_LEN+1 ; i++ )
    {
        /*
        *  If previous iteration failed to find a bit,
        *  insert UNKNOWN in tty_bit_hist[]
        */

        *bit_index = add(i,1);

        if( sub(ditbuf[i],ERASE) != 0 )
        {
            fail_counter = add(fail_counter,1);

            tty_bit_hist[STOP_BIT_IDX] = UNKNOWN;                   
            tty_bit_len_hist[STOP_BIT_IDX] = fail_counter;
        }
                                                                    
        /*
        *  If there is a transition in the dits
        *  count mark, space, and unknown dits in
        *  a window and form TTY bits.
        */
                                                    
        if( sub(ditbuf[i],ditbuf[i+1]) != 0 )
        {
            mark_counter = 0;                       
            space_counter = 0;                      
            good_dits = 0;                          
            bad_dits = 0;                           
            tty_unknown_counter = 0;                
            for( j=i+1 ; j < MIN(DITBUF_LEN,MAX_BIT_LEN+i+2) ; j++ )
            {
                /* Count LOGIC_1, LOGIC_0, and UNKNOWN dits */
                                                    
                if( sub(ditbuf[j],LOGIC_0) == 0 )
                {
                    space_counter = add(space_counter,1);
                }
                else if( sub(ditbuf[j],LOGIC_1) == 0 )
                {
                    mark_counter = add(mark_counter,1);                 
                }
                else
                {
                    tty_unknown_counter = add(tty_unknown_counter,1);          
                }

                /* Check if there are more 1's or 0's in the dit window */
                                                    
                if( sub(space_counter,mark_counter) > 0 )
                {
                    bit = LOGIC_0;                  
                    good_dits = space_counter;      
                    bad_dits = mark_counter;        
                }
                else
                {
                    bit = LOGIC_1;                  
                    good_dits = mark_counter;       
                    bad_dits = space_counter;       
                }


                /* If the number of UNKNOWN's is too high, abort */
                                                    
                if( ( sub(tty_unknown_counter,TTY_BIT_UNKNOWN_THRESH) > 0
                     || sub(bad_dits,BAD_DIT_THRESH) > 0 )
                    && sub(good_dits,GOOD_DIT_THRESH) < 0
                  )
                {
                    break;
                }

                /*
                *  If the new bit is different from the previous bit
                *  and the new bit starts with dits from the previous
                *  bit, break so that the bit boundaries will resync.
                *  in the logic below.
                */

                if( sub(bit,tty_bit_hist[STOP_BIT_IDX]) != 0
                    && ( sub(ditbuf[i+1],tty_bit_hist[STOP_BIT_IDX]) == 0
                        || ( sub(bit,ditbuf[i+1]) != 0
                             && ( sub(ditbuf[i+1],LOGIC_0) == 0
                                  || sub(ditbuf[i+1],LOGIC_1) == 0 ))
                       )
                  )
                {
                    break;
                }

                /*
                *  If the number of mark or space detections is above
                *  a minimum threshold...
                */

                len = sub(j,i);
                                                                    
                if( sub(good_dits,GOOD_DIT_THRESH) >= 0
                    && sub(bad_dits,BAD_DIT_THRESH) <= 0
                    && sub(len,MIN_BIT_LEN) >= 0
                  )
                {
                    /* If there is a break in the run of dits... */
                                                                    
                    if( sub(bit,ditbuf[j]) != 0 )
                    {
                        /*
                        *  If the run is broken by a mark or space,
                        *  declare the current bit.
                        */
                                                                    
                        if( sub(ditbuf[j],LOGIC_0) == 0
                            || sub(ditbuf[j],LOGIC_1) == 0
                            || sub(tty_unknown_counter,TTY_BIT_UNKNOWN_THRESH) > 0
                          )
                        {
                            /* Subtract the last dit from the counters */
                            len = sub(len,1);
                            if( sub(len,MIN_BIT_LEN) < 0 )
                            {
                                break;
                            }


                            bit_len = len;
                            bad_dits = sub(bad_dits,1);
                            *bit_index = sub(j,1);                       
                            update_flag = 1;                        
                            break;
                        }
                    }
                                                                    
                    if( sub(len,MAX_BIT_LEN) > 0 )
                    {
                                                                    
                        if( sub(baud_rate,TTY_45_BAUD) == 0 )
                        {
                            bit_len = BIT_LEN_45_BAUD;              
                        }
                        else
                        {
                            bit_len = BIT_LEN_50_BAUD;              
                        }

                        /*
                        *  Subtract the unknown/bad dits from the counter if
                        *  they are beyond bit_len.
                        */
                        for( k=0 ; k <= (MAX_BIT_LEN-bit_len) ; k++ )
                        {
                                                                    
                            if( sub(ditbuf[i+k+bit_len+1],bit) != 0 )
                            {
                                tty_unknown_counter = sub(tty_unknown_counter,1);
                            }
                            else
                            {
                                good_dits = sub(good_dits,1);
                            }
                        }

                        if( sub(good_dits,GOOD_DIT_THRESH) < 0 )
                        {
                            bit_len = 0;
                            break;
                        }

                        *bit_index = add(bit_len,i);                     
                        update_flag = 1;                            

                        break;
                    }
                }

                /*
                *  If the end of the ditbuf came before the end of a
                *  run was found, but the partial bit result in the
                *  last element of tty_bit_hist[]
                */

                if( sub(j,DITBUF_LEN-1) == 0 )
                {
                    bit_len = len;
                    tty_bit_hist[TTY_BIT_HIST_LEN-1] = bit;         
                    tty_bit_len_hist[TTY_BIT_HIST_LEN-1] = bit_len; 
                    *bit_index = i;                                 
                    break;
                }

            } /* end for(j) */

            /* If a TTY bit was detected */
                                                                    
            if( bit_len > 0 )
            {
                break;
            }
            else if( ( sub(ditbuf[i+1],LOGIC_0) == 0
                       || sub(ditbuf[i+1],LOGIC_1) == 0)
                     && ( sub(ditbuf[i+1],tty_bit_hist[STOP_BIT_IDX]) == 0 )
                   )
            {
                /*
                *  Resync the bit boundaries if a bit wasn't found but
                *  mark or space detections from the previous bit
                *  were found.
                */
                                                                        

                if( sub(baud_rate,TTY_50_BAUD) == 0
                    || sub(tty_bit_len_hist[STOP_BIT_IDX],BIT_LEN_45_BAUD) < 0 )
                {
                    /*
                    *  If the baud rate is 50 or the bit_len is less than a nominal
                    *  45 baud bit, add the extra dits to last bit.  This helps switch
                    *  from 50 baud to 45 baud.
                    */
                    tty_bit_len_hist[STOP_BIT_IDX] = add(tty_bit_len_hist[STOP_BIT_IDX],1);
                    ditbuf[i+1] = ERASE;                                
                    fail_counter = 0;                                   
                    *bit_index = add(i,1);                                   
                }
                else
                {
                    /*
                    *  If the baud rate is 45 baud, use the extra dits to start a new
                    *  bit, making it easier to switch into 50 baud.
                    */
                    bit = ditbuf[i+1];
                    bit_len = 1;
                    bad_dits = 0;
                    *bit_index = add(i,1);                       
                    fail_counter = 0;                       
                    update_flag = 1;                        
                    break;
                }
            }


        } /* end if( ditbuf[i] != ditbuf[i+1] ) */


    } /* end for(i) */



                                                                        
    /* Erase dits that were counted as part of a bit */

    for( i=0 ; i <= *bit_index ; i++ )
    {
        ditbuf[i] = ERASE;
    }


    /* Update history buffers */

    if( update_flag == 0 )
    {
        *bit_index = add(*bit_index,bit_len);
        tty_unknown_counter = 0;                    
        bad_dits = 0;                               
    }
    else
    {
        /* Update tty_bit_hist[] */

        for( i=1 ; i < TTY_BIT_HIST_LEN-1 ; i++ )
        {
            tty_bit_hist[i-1] = tty_bit_hist[i];                
        }

        /* Update tty_bit_len_hist[] */

        for( i=2 ; i < TTY_BIT_HIST_LEN-1 ; i++ )
        {
            tty_bit_len_hist[i-1] = tty_bit_len_hist[i];        
        }
        
        /* Update history with newest bit */
        
        tty_bit_hist[STOP_BIT_IDX] = bit;                       
        tty_bit_len_hist[STOP_BIT_IDX] = bit_len;               

        tty_bit_hist[TTY_BIT_HIST_LEN-1] = ERASE;               
        tty_bit_len_hist[TTY_BIT_HIST_LEN-1] = 0;               
    }



    /*
    *  Zero the unknown counter for the character
    *  if a LOGIC_0 is in the memory bit.
    */

    if( sub(tty_bit_hist[MEM_BIT_IDX],LOGIC_0) == 0 )
    {
        tty_bit_len_hist[MEM_BIT_IDX] = 0;
    }


    /* Update the total # of bad and unknown dits  */

    tty_bit_len_hist[MEM_BIT_IDX] = add(tty_bit_len_hist[MEM_BIT_IDX],tty_unknown_counter);
    tty_bit_len_hist[MEM_BIT_IDX] = add(tty_bit_len_hist[MEM_BIT_IDX],bad_dits);



    return(update_flag);

} /* end get_tty_bit() */

