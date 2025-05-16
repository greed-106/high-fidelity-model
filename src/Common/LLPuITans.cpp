/* ====================================================================================================================

  The copyright in this software is being made available under the License included below.
  This software may be subject to other third party and contributor rights, including patent rights, and no such
  rights are granted under this license.

  Copyright (c) 2025, HUAWEI TECHNOLOGIES CO., LTD. All rights reserved.
  Copyright (c) 2025, PEKING UNIVERSITY. All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted only for
  the purpose of developing standards within Audio and Video Coding Standard Workgroup of China (AVS) and for testing and
  promoting such standards. The following conditions are required to be met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and
      the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and
      the following disclaimer in the documentation and/or other materials provided with the distribution.
    * The name of HUAWEI TECHNOLOGIES CO., LTD. may not be used to endorse or promote products derived from
      this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
  INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

* ====================================================================================================================
*/
#include "LLPuITrans.h"

namespace HFM {
    LLPuITrans::LLPuITrans() {
        coeffITransVer_.resize(MB_SIZE*MB_SIZE, 0);
    }
    
    LLPuITrans::~LLPuITrans() {
    }

    void LLPuITrans::Set(uint8_t puWidth, uint8_t puHeight, uint8_t predMode) {
        puWidth_ = puWidth;
        puHeight_ = puHeight;
#if DST
        horITransMode_ = ((puWidth == 4) && (predMode == INTRA_HOR || predMode == INTRA_PLANAR) ? DST7 : DCT2);
        verITransMode_ = ((puHeight == 4) && (predMode == INTRA_VER || predMode == INTRA_PLANAR) ? DST7 : DCT2);
#else
        horITransMode_ = DCT2;
        verITransMode_ = DCT2;
#endif
    }

    void LLPuITrans::DST7W4(std::vector<int32_t>& src, std::vector<int32_t>& dst, uint8_t length, uint8_t shift) {
        int32_t offset = 1 << (shift - 1);
        int32_t dstStep1[4];
        auto ptrSrc = &src[0];
        auto ptrDst = &dst[0];
        for (uint8_t currentLine = 0; currentLine < length; currentLine++) {
            dstStep1[0] = ptrSrc[0 * length] + ptrSrc[2 * length];
            dstStep1[1] = ptrSrc[2 * length] + ptrSrc[3 * length];
            dstStep1[2] = ptrSrc[0 * length] - ptrSrc[3 * length];
            dstStep1[3] = DST7_W4[0][2] * ptrSrc[1 * length];
            ptrDst[0] = (int32_t)((DST7_W4[0][0] * dstStep1[0] + DST7_W4[0][1] * dstStep1[1] + dstStep1[3] + offset) >> shift);
            ptrDst[1] = (int32_t)((DST7_W4[0][1] * dstStep1[2] - DST7_W4[0][0] * dstStep1[1] + dstStep1[3] + offset) >> shift);
            ptrDst[2] = (int32_t)((DST7_W4[0][2] * (ptrSrc[0 * length] - ptrSrc[2 * length] + ptrSrc[3 * length]) + offset) >> shift);
            ptrDst[3] = (int32_t)((DST7_W4[0][1] * dstStep1[0] + DST7_W4[0][0] * dstStep1[2] - dstStep1[3] + offset) >> shift);
            ptrSrc++;
            ptrDst += 4;

        }
    }

    void LLPuITrans::DCT2W4(std::vector<int32_t>& src, std::vector<int32_t>& dst, uint8_t length, uint8_t shift) {
        int32_t offset = 1 << (shift - 1);
        int32_t dctStep1Com1[2];
        int32_t dctStep1Com2[2];
        auto ptrSrc = &src[0];
        auto ptrDst = &dst[0];
        for (uint8_t currentLine = 0; currentLine < length; currentLine++) {
            dctStep1Com1[0] = DCT2_W4[1][0] * ptrSrc[length + currentLine] + DCT2_W4[3][0] * ptrSrc[3 * length + currentLine];
            dctStep1Com1[1] = DCT2_W4[1][1] * ptrSrc[length + currentLine] + DCT2_W4[3][1] * ptrSrc[3 * length + currentLine];
            dctStep1Com2[0] = DCT2_W4[0][0] * ptrSrc[currentLine] + DCT2_W4[2][0] * ptrSrc[2 * length + currentLine];
            dctStep1Com2[1] = DCT2_W4[0][1] * ptrSrc[currentLine] + DCT2_W4[2][1] * ptrSrc[2 * length + currentLine];

            ptrDst[4 * currentLine + 0] = (int32_t)((dctStep1Com1[0] + dctStep1Com2[0] + offset) >> shift);
            ptrDst[4 * currentLine + 1] = (int32_t)((dctStep1Com1[1] + dctStep1Com2[1] + offset) >> shift);
            ptrDst[4 * currentLine + 2] = (int32_t)((dctStep1Com2[1] - dctStep1Com1[1] + offset) >> shift);
            ptrDst[4 * currentLine + 3] = (int32_t)((dctStep1Com2[0] - dctStep1Com1[0] + offset) >> shift);
        }

    }

    void LLPuITrans::DCT2W8(std::vector<int32_t>& src, std::vector<int32_t>& dst, uint8_t length, uint8_t shift) {
        int32_t offset = 1 << (shift - 1);
        int32_t dctStep1Com1[4];
        int32_t dctStep1Com2[4];
        int32_t dctStep2Com1[2];
        int32_t dctStep2Com2[2];
        auto ptrSrc = &src[0];
        auto ptrDst = &dst[0];
        for (uint8_t currentLine = 0; currentLine < length; currentLine++) {
            for (uint8_t currentPix = 0; currentPix < 4; currentPix++) {
                dctStep1Com1[currentPix] = DCT2_W8[1][currentPix] * src[1 * length + currentLine] + DCT2_W8[3][currentPix] * src[3 * length + currentLine]
                    + DCT2_W8[5][currentPix] * src[5 * length + currentLine] + DCT2_W8[7][currentPix] * src[7 * length + currentLine];
            }

            dctStep2Com1[0] = DCT2_W8[2][0] * src[2 * length + currentLine] + DCT2_W8[6][0] * src[6 * length + currentLine];
            dctStep2Com1[1] = DCT2_W8[2][1] * src[2 * length + currentLine] + DCT2_W8[6][1] * src[6 * length + currentLine];
            dctStep2Com2[0] = DCT2_W8[0][0] * src[0 * length + currentLine] + DCT2_W8[4][0] * src[4 * length + currentLine];
            dctStep2Com2[1] = DCT2_W8[0][1] * src[0 * length + currentLine] + DCT2_W8[4][1] * src[4 * length + currentLine];
            
            dctStep1Com2[0] = dctStep2Com2[0] + dctStep2Com1[0];
            dctStep1Com2[3] = dctStep2Com2[0] - dctStep2Com1[0];
            dctStep1Com2[1] = dctStep2Com2[1] + dctStep2Com1[1];
            dctStep1Com2[2] = dctStep2Com2[1] - dctStep2Com1[1];
            for (uint8_t currentPix = 0; currentPix < 4; currentPix++) {
                ptrDst[currentLine * 8 + currentPix] = (dctStep1Com2[currentPix] + dctStep1Com1[currentPix] + offset) >> shift;
                ptrDst[currentLine * 8 + currentPix + 4] = (dctStep1Com2[3 - currentPix] - dctStep1Com1[3 - currentPix] + offset) >> shift;
            }
        }
    }


    void LLPuITrans::ITrans(std::vector<int32_t>& src, std::vector<int32_t>& dst, uint8_t is8, TRANS_TYPE transMode, uint8_t length, uint8_t shift) {
        if (transMode == DST7) {
            if (is8) { exit(-1); }
            DST7W4(src, dst, length, shift);
        } else {
            if (is8) {
                DCT2W8(src, dst, length, shift);
            } else {
                DCT2W4(src, dst, length, shift);
            }
        }
    }

    void LLPuITrans::ComLLITrans(std::vector<int32_t>& coeff) {
        uint8_t isWidth8 = (puWidth_ == 4 ? 0 : 1);
        uint8_t isHeight8 = (puHeight_ == 4 ? 0 : 1);
        uint8_t ver_shift = LL_ITRANS1_SHIFT;
        uint8_t hor_shift = LL_ITRANS2_SHIFT;
        int32_t maxITransVal = (1 << LL_ITRANS1_DYNAMIC_BIT) - 1;
        int32_t minITransVal = -(1 << LL_ITRANS1_DYNAMIC_BIT);

        ITrans(coeff, coeffITransVer_, isHeight8, verITransMode_, puWidth_, ver_shift);
        for (uint8_t i = 0; i < puWidth_*puHeight_; i++) {
            Clip<int32_t, int32_t>(coeffITransVer_[i], coeffITransVer_[i], minITransVal, maxITransVal);
        }

        maxITransVal = (1 << LL_ITRANS2_DYNAMIC_BIT) - 1;
        minITransVal = -(1 << LL_ITRANS2_DYNAMIC_BIT);
        ITrans(coeffITransVer_, coeff, isWidth8, horITransMode_, puHeight_, hor_shift);
        for (uint8_t i = 0; i < puWidth_*puHeight_; i++) {
            Clip<int32_t, int32_t>(coeff[i], coeff[i], minITransVal, maxITransVal);
        }

    }
}

