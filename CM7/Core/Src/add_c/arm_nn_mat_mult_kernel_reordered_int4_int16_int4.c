/* Copyright (C) 2020 University of Bologna
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the License); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

 /******************************************************************
 * Project:      CMixNN Inference Library
 * Title:        arm_nn_mat_mult_kernel_reordered_u4_int16_u4.c
 * Description:  Matrix-Multiplication function for
 *               u4 x int16_t convolution with reordered columns.
 *               Output is then quantized to u4 using weights
 *               config.folding technique.
  * Target:       ARM Cortex-M cores
 *
 * Date:         10 February 2020
 * Revision:     Release v1.0.0
 *
 * Authors:      Alessandro Capotondi
 *                 <alessandro.capotondi AT unibo.it>
 *               Marco Fariselli
 *                 <marco.fariselli AT greenwaves-technologies.com>
 *               Manuele Rusci
 *                 <manuele.rusci AT unibo.it>
 *
 ******************************************************************/

#include "arm_cmixnn.h"
#include "arm_nnfunctions.h"
#include "arm_nnsupportfunctions.h"
#include "arm_math.h"

  /**
   * @brief Matrix-Multiplication function for u4 x int16_t convolution with reordered columns.
   *        Output is then quantized to u4 using weights config.folding technique.
   * @param[in]       pA          pointer to operand A
   * @param[in]       pInBuffer   pointer to operand B, always consists of 2 vectors
   * @param[in]       ch_im_out   numRow of A
   * @param[in]       numCol_A    numCol of A
   * @param[in]       bias        the bias
   * @param[in,out]   pOut        pointer to output
   * @param[in]       z_a         A operand offset
   * @param[in]       z_a         A operand offset
   * @param[in]       z_out       output offset
   * @param[in]       m_zero      m zero quantization param
   * @param[in]       n_zero      n zero quantization param
   * @return     The function returns the incremented output pointer
   *
   * @details
   *
   * This function assumes that data in pInBuffer are reordered
   */

uint8_t
*arm_nn_mat_mult_kernel_reordered_int4_int16_int4(const uint8_t * pA,
                            const int16_t * pInBuffer,
                            const uint16_t ch_im_out,
                            const uint16_t numCol_A,
                            const int32_t * bias,
                            uint8_t * pOut,
                            const uint8_t z_a,
                            const uint8_t z_out,
                            const int32_t *m_zero,
                            const int8_t *n_zero)
{

#if defined (ARM_MATH_DSP)
    /* set up the second output pointers */
    uint8_t *pOut2 = pOut + (ch_im_out>>1); // config.out_data_t: u4 (2CHs per-Bytes)
    int     i;
    const int16_t *pB = pInBuffer;
    const int16_t *pB2 = pB + numCol_A;

    /* Negative N_ZERO Normalization */
    int8_t n_zero1;
    int8_t n_zero2;
   // __n_zero_negative_normalization(n_zero,&n_zero1,&n_zero2);

    int16_t VzA[2] = {z_a,z_a};
    const int16_t *pzA = VzA;
    int32_t inzA = *__SIMD32(pzA);

    /* Pre-compute z_a offset over the inputs */
    int32_t z_a_offset  = 0;
    int32_t z_a_offset2 = 0;


    for (i = 0; i < numCol_A; i += 2) {
        int32_t inB1 = *__SIMD32(pB)++;
        int32_t inB2 = *__SIMD32(pB2)++;
        z_a_offset = __SMLAD(inzA, inB1, z_a_offset);
        z_a_offset2 = __SMLAD(inzA, inB2, z_a_offset2);
    }

    /* Leftover column */
    if (numCol_A & 0x1)
    {
        int16_t inB1 = *pB;
        int16_t inB2 = *pB2;
        z_a_offset += inB1*z_a;
        z_a_offset2 += inB2*z_a;
    }

    /* this loop over rows in A */
    for (i = 0; i < ch_im_out; i += 2)
    {
        /* setup pointers for B */
        pB = pInBuffer;
        pB2 = pB + numCol_A;

        /* align the second pointer for A */
        const uint8_t *pA2 = pA + (numCol_A>>1); // config.wt_data_t: u4 (4Cols per-Byte)

//        int32_t     sum =  bias[i] - z_a_offset;
//        int32_t     sum2 = bias[i] - z_a_offset2;
//        int32_t     sum3 = bias[i + 1] - z_a_offset;
//        int32_t     sum4 = bias[i + 1] - z_a_offset2;
        int32_t sum = 0;
        int32_t sum2 =0;
        int32_t res1 = bias[i] - z_a_offset;
        int32_t res2 = bias[i] - z_a_offset2;
        int32_t res3 = bias[i + 1] - z_a_offset;
        int32_t res4 = bias[i + 1] - z_a_offset2;
        uint16_t val1 =0;
        uint16_t val2 =0;
        uint16_t  colCnt = numCol_A >> 3; // config.wt_data_t: u4 (8x uint4_t)

        /* accumulate over the vector */
        while (colCnt)
        {
            int32_t inA11, inA12, inA21, inA22;
            int32_t inA13, inA14, inA23, inA24;
            int32_t inB;

            int32_t inB1 = *__SIMD32(pB)++;//卷积核第一步覆盖的输入数据
            int32_t inB2 = *__SIMD32(pB2)++;//第二步
            inB = inB1 | (inB2 << 10);

            pA = (uint8_t *) read_and_pad_reordered_u4((void *)pA, &inA11, &inA12, &inA13, &inA14);//第一个卷积核第一个位置的8个通道
            pA2 = (uint8_t *) read_and_pad_reordered_u4((void *)pA2, &inA21, &inA22, &inA23, &inA24); //第二个卷积核第一个位置的8个通道

//            sum = __SMLAD(inA11, inB1, sum);//第一个卷积核第一步部分乘积
//            sum2 = __SMLAD(inA11, inB2, sum2);//第一个卷积核第二步部分乘积
//            sum3 = __SMLAD(inA21, inB1, sum3);//第二个卷积核第一步部分乘积
//            sum4 = __SMLAD(inA21, inB2, sum4);//第二个卷积核第二步部分乘积
            sum = __SMLAD(inA11, inB,sum);
            sum2 = __SMLAD(inA21, inB,sum2);
            //sum = __SMUAD(inA11, inB);//此时sum中有第一个卷积核的第一二步部分乘积 构成：0x000第二步(8bit)F第一步(8bit)
            //sum2 = __SMUAD(inA21, inB);//此时sum2中有第二个卷积核的第一二步部分乘积 构成：0x000第二步(8bit)F第一步(8bit)
            //result_reordered_int4(&sum, &res1); //经过处理res1中的0-15位存放第一个卷积核的第一步的乘累加和，16-31存放第二步的乘累加和，也就是输出特征图上第一二个位置的元素
            //result_reordered_int4(&sum2, &res2);//经过处理res2中的0-15位存放第二个卷积核的第一步的乘累加和，16-31存放第二步的乘累加和，也就是输出特征图上第一二个位置的元素



            inB1 = *__SIMD32(pB)++;
            inB2 = *__SIMD32(pB2)++;
            inB = inB1 | (inB2 << 10);//12

//            sum = __SMLAD(inA12, inB1, sum);
//            sum2 = __SMLAD(inA12, inB2, sum2);
//            sum3 = __SMLAD(inA22, inB1, sum3);
//            sum4 = __SMLAD(inA22, inB2, sum4);
            sum = __SMLAD(inA12, inB,sum);
            sum2 = __SMLAD(inA22, inB,sum2);
            //sum = __SMUAD(inA12, inB);
            //sum2 = __SMUAD(inA22, inB);
//            result_reordered_int4(&sum, &res1, &res2);
//            result_reordered_int4(&sum2, &res3, &res4);
//            sum = 0;
//            sum2 = 0;


            inB1 = *__SIMD32(pB)++;
            inB2 = *__SIMD32(pB2)++;
            inB = inB1 | (inB2 << 10);

//            sum = __SMLAD(inA13, inB1, sum);
//            sum2 = __SMLAD(inA13, inB2, sum2);
//            sum3 = __SMLAD(inA23, inB1, sum3);
//            sum4 = __SMLAD(inA23, inB2, sum4);
            sum = __SMLAD(inA13, inB,sum);
            sum2 = __SMLAD(inA23, inB,sum2);
            //sum = __SMUAD(inA13, inB);
            //sum2 = __SMUAD(inA23, inB);
            //result_reordered_int4(&sum, &res1);
            //result_reordered_int4(&sum2, &res2);


//            sum = __SMLAD(inA14, inB1, sum);
//            sum2 = __SMLAD(inA14, inB2, sum2);
//            sum3 = __SMLAD(inA24, inB1, sum3);
//            sum4 = __SMLAD(inA24, inB2, sum4);
            inB1 = *__SIMD32(pB)++;
            inB2 = *__SIMD32(pB2)++;
            inB = inB1 | (inB2 << 10);
            sum = __SMLAD(inA14, inB,sum);
            sum2 = __SMLAD(inA24, inB,sum2);

            val1 = (uint16_t)(sum & 0x3FF);
            val2 = (uint16_t)(sum2 & 0x3FF);
            //res1 += (int16_t)(sum & 0x3FF);
            res1 += val1;
            //res2 += (int16_t)((sum >> 10) & 0x3FF);
            //res3 += (int16_t)(sum2 & 0x3FF);
            res3 += val2;
            //res4 += (int16_t)((sum2 >> 10) & 0x3FF);
            //result_reordered_uint4(&sum, &res1, &res2);
            //result_reordered_uint4(&sum2, &res3, &res4);
            //sum = 0;
            //sum2 = 0;
//            sum  -= (uint16_t)(sum  & 0x3FF);
//            sum2 -= (uint16_t)(sum2 & 0x3FF);

            sum = __SSUB16(sum, val1);
            sum2 = __SSUB16(sum2, val2);
           //result_reordered_uint4(&sum, &sum2, &res1, &res3);


            colCnt--;
        } /* while over colCnt */

#if 0
        colCnt = numCol_A & 0x7;; // config.wt_data_t: u4 (8x uint4_t)

        int wt_per_byte = 2;
        while (colCnt)
        {
            uint8_t inB1 = (uint8_t) *pB++;
            uint8_t inA1;
            switch(wt_per_byte)
            {
                case 2:
                    inA1 = (uint8_t) __USAT(*pA, 4);
                    break;
                 case 1:
                    inA1 = (uint8_t) __USAT(__ROR(*pA, 4), 4);
                    pA++;
                    break;
            }
            inA1 -= z_a;
            sum += inA1 * inB1;
            colCnt--;
        }
#endif
        res2 += (uint32_t)(sum >> 10);
        res4 += (uint32_t)(sum2 >> 10);
        /* Normalize by PACT+FW (u4 output) */
//        sum  = ((__HI_SMULL(sum << n_zero1,m_zero)) >> n_zero2) + z_out;
//        sum2 = ((__HI_SMULL(sum2 << n_zero1,m_zero)) >> n_zero2) + z_out;
//        sum3 = ((__HI_SMULL(sum3 << n_zero1,m_zero)) >> n_zero2) + z_out;
//        sum4 = ((__HI_SMULL(sum4 << n_zero1,m_zero)) >> n_zero2) + z_out;


        __n_zero_negative_normalization(n_zero[i],&n_zero1,&n_zero2);
        res1  = ((__HI_SMULL(res1 << n_zero1 ,m_zero[i])) >> n_zero2) + z_out;
        __n_zero_negative_normalization(n_zero[i],&n_zero1,&n_zero2);
        res2  = ((__HI_SMULL(res2 << n_zero1 ,m_zero[i])) >> n_zero2) + z_out;
        __n_zero_negative_normalization(n_zero[i+1],&n_zero1,&n_zero2);
        res3  = ((__HI_SMULL(res3 << n_zero1 ,m_zero[i+1])) >> n_zero2) + z_out;
        __n_zero_negative_normalization(n_zero[i+1],&n_zero1,&n_zero2);
        res4  = ((__HI_SMULL(res4 << n_zero1 ,m_zero[i+1])) >> n_zero2) + z_out;
//        res1 = arm_nn_requantize(res1, n_zero1, n_zero2);
//        res2 = arm_nn_requantize(res2, n_zero1, n_zero2);
//        res3 = arm_nn_requantize(res3, n_zero1, n_zero2);
//        res4 = arm_nn_requantize(res4, n_zero1, n_zero2);

        /* Store Outputs (u4 output) */
        //        *pOut++  = ( __USAT(sum,4) | ((__USAT(s9*um3,4) << 4 ) & 0xF0 ));  // 按照HWC存储输出，sum是第一个输出通道的第一个结果，sum3是第二个输出通道的第一个结果
//        *pOut2++ = ( __USAT(sum2,4) | ((__USAT(sum4,4) << 4 ) & 0xF0 )); //按照HWC存储输出，sum2是第一个输出通道的第二个结果，sum3是第二个输出通道的第二个结果 __SSAT截取到有符号数
        //result_reordered_int4(&sum, &res1);
        //result_reordered_int4(&sum2, &res2);
        *pOut++  = ( __USAT(res1,4) | ((__USAT(res3,4) << 4 ) & 0xF0 ));
        *pOut2++ = ( __USAT(res2,4) | ((__USAT(res4,4) << 4 ) & 0xF0 ));
       //4.30没有offset //*pOut++  = ( __USAT((res1 & 0xFFFF),4) | ((__USAT((res2 & 0xFFFF),4) << 4 ) & 0xF0 ));
        //*pOut2++ = ( __USAT(((res2 >> 16) & 0xFFFF),4) | ((__USAT(((res2 >> 16) & 0xFF),4) << 4 ) & 0xF0 ));
        /* skip the row computed with A2 */
        pA += numCol_A>>1; // config.wt_data_t: u4 (2cols per-Bytes)
    } /* for over ch_im_out */

    pOut += ch_im_out>>1; // config.out_data_t: u4 (2CH per-Bytes)
#else
    #error "Cortex-M0 and Cortex-M3 not supported"
    /* Run the following code as reference implementation for Cortex-M0 and Cortex-M3 */
#endif /* ARM_MATH_DSP */

    /* return the new output pointer with offset */
    return pOut;
}
