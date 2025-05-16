/* ====================================================================================================================

Copyright(c) 2025, HUAWEI TECHNOLOGIES CO., LTD.
Licensed under the Code Sharing Policy of the UHD World Association(the "Policy");
http://www.theuwa.com/UWA_Code_Sharing_Policy.pdf.
you may not use this file except in compliance with the Policy.
Unless agreed to in writing, software distributed under the Policy is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OF ANY KIND, either express or implied.
See the Policy for the specific language governing permissions and
limitations under the Policy.

* ====================================================================================================================
*/
#include "LLEncoderTrans.h"

namespace ProVivid {
    LLEncoderTrans::LLEncoderTrans() {
        coeffTransHor_.resize(MB_SIZE*MB_SIZE, 0);
    }

    LLEncoderTrans::~LLEncoderTrans() {
    }

    void LLEncoderTrans::Set(uint8_t puWidth, uint8_t puHeight, uint8_t predMode) {
        this->puWidth_ = puWidth; this->puHeight_ = puHeight;
#if DST
        horTransMode_ = ((puWidth == 4) && (predMode == INTRA_HOR || predMode == INTRA_PLANAR) ? DST7 : DCT2);
        verTransMode_ = ((puHeight == 4) && (predMode == INTRA_VER || predMode == INTRA_PLANAR) ? DST7 : DCT2);
#else
        horTransMode_ = DCT2;
        verTransMode_ = DCT2;
#endif
    }



    void LLEncoderTrans::DST7W4(FrameBuffer& src, FrameBuffer& dst, uint8_t length, uint8_t shift) {
        int32_t offset = 1 << (shift - 1);
        int32_t dstStep1[4];
        auto ptrSrc = &src[0];
        auto ptrDst = &dst[0];
        for (uint8_t currentLine = 0; currentLine < length; currentLine++) {
            dstStep1[0] = ptrSrc[0] + ptrSrc[3];
            dstStep1[1] = ptrSrc[1] + ptrSrc[3];
            dstStep1[2] = ptrSrc[0] - ptrSrc[1];
            dstStep1[3] = DST7_W4[0][2] * ptrSrc[2];
            ptrDst[0 * length] = (int32_t)((DST7_W4[0][0] * dstStep1[0] + DST7_W4[0][1] * dstStep1[1] + dstStep1[3] + offset) >> shift);
            ptrDst[1 * length] = (int32_t)((DST7_W4[0][2] * (ptrSrc[0] + ptrSrc[1] - ptrSrc[3]) + offset) >> shift);
            ptrDst[2 * length] = (int32_t)((DST7_W4[0][0] * dstStep1[2] + DST7_W4[0][1] * dstStep1[0] - dstStep1[3] + offset) >> shift);
            ptrDst[3 * length] = (int32_t)((DST7_W4[0][1] * dstStep1[2] - DST7_W4[0][0] * dstStep1[1] + dstStep1[3] + offset) >> shift);
            ptrSrc += 4;
            ptrDst++;

        }
    }

    void LLEncoderTrans::DCT2W4(FrameBuffer& src, FrameBuffer& dst, uint8_t length, uint8_t shift) {
        int32_t offset = 1 << (shift - 1);
        int32_t dctStep1Com1[2];
        int32_t dctStep1Com2[2];
        auto ptrSrc = &src[0];
        auto ptrDst = &dst[0];
        for (uint8_t currentLine = 0; currentLine < length; currentLine++) {
            dctStep1Com1[0] = ptrSrc[currentLine * 4 + 0] + ptrSrc[currentLine * 4 + 3];
            dctStep1Com2[0] = ptrSrc[currentLine * 4 + 0] - ptrSrc[currentLine * 4 + 3];
            dctStep1Com1[1] = ptrSrc[currentLine * 4 + 1] + ptrSrc[currentLine * 4 + 2];
            dctStep1Com2[1] = ptrSrc[currentLine * 4 + 1] - ptrSrc[currentLine * 4 + 2];

            ptrDst[0 * length + currentLine] = (int32_t)((DCT2_W4[0][0] * dctStep1Com1[0] + DCT2_W4[0][1] * dctStep1Com1[1] + offset) >> shift);
            ptrDst[2 * length + currentLine] = (int32_t)((DCT2_W4[2][0] * dctStep1Com1[0] + DCT2_W4[2][1] * dctStep1Com1[1] + offset) >> shift);
            ptrDst[1 * length + currentLine] = (int32_t)((DCT2_W4[1][0] * dctStep1Com2[0] + DCT2_W4[1][1] * dctStep1Com2[1] + offset) >> shift);
            ptrDst[3 * length + currentLine] = (int32_t)((DCT2_W4[3][0] * dctStep1Com2[0] + DCT2_W4[3][1] * dctStep1Com2[1] + offset) >> shift);
        }

    }

    void LLEncoderTrans::DCT2W8(FrameBuffer& src, FrameBuffer& dst, uint8_t length, uint8_t shift) {
        int32_t offset = 1 << (shift - 1);
        int32_t dctStep1Com1[4];
        int32_t dctStep1Com2[4];
        int32_t dctStep2Com1[2];
        int32_t dctStep2Com2[2];
        auto ptrSrc = &src[0];
        auto ptrDst = &dst[0];
        for (uint8_t currentLine = 0; currentLine < length; currentLine++) {
            for (uint8_t currentPix = 0; currentPix < 4; currentPix++) {
                dctStep1Com1[currentPix] = ptrSrc[currentLine * 8 + currentPix] + ptrSrc[currentLine * 8 + 7 - currentPix];
                dctStep1Com2[currentPix] = ptrSrc[currentLine * 8 + currentPix] - ptrSrc[currentLine * 8 + 7 - currentPix];
            }

            dctStep2Com1[0] = dctStep1Com1[0] + dctStep1Com1[3];
            dctStep2Com2[0] = dctStep1Com1[0] - dctStep1Com1[3];
            dctStep2Com1[1] = dctStep1Com1[1] + dctStep1Com1[2];
            dctStep2Com2[1] = dctStep1Com1[1] - dctStep1Com1[2];

            ptrDst[0 * length + currentLine] = (int32_t)((DCT2_W8[0][0] * dctStep2Com1[0] + DCT2_W8[0][1] * dctStep2Com1[1] + offset) >> shift);
            ptrDst[4 * length + currentLine] = (int32_t)((DCT2_W8[4][0] * dctStep2Com1[0] + DCT2_W8[4][1] * dctStep2Com1[1] + offset) >> shift);
            ptrDst[2 * length + currentLine] = (int32_t)((DCT2_W8[2][0] * dctStep2Com2[0] + DCT2_W8[2][1] * dctStep2Com2[1] + offset) >> shift);
            ptrDst[6 * length + currentLine] = (int32_t)((DCT2_W8[6][0] * dctStep2Com2[0] + DCT2_W8[6][1] * dctStep2Com2[1] + offset) >> shift);
            ptrDst[1 * length + currentLine] = (int32_t)((DCT2_W8[1][0] * dctStep1Com2[0] + DCT2_W8[1][1] * dctStep1Com2[1] + DCT2_W8[1][2] * dctStep1Com2[2] + DCT2_W8[1][3] * dctStep1Com2[3] + offset) >> shift);
            ptrDst[3 * length + currentLine] = (int32_t)((DCT2_W8[3][0] * dctStep1Com2[0] + DCT2_W8[3][1] * dctStep1Com2[1] + DCT2_W8[3][2] * dctStep1Com2[2] + DCT2_W8[3][3] * dctStep1Com2[3] + offset) >> shift);
            ptrDst[5 * length + currentLine] = (int32_t)((DCT2_W8[5][0] * dctStep1Com2[0] + DCT2_W8[5][1] * dctStep1Com2[1] + DCT2_W8[5][2] * dctStep1Com2[2] + DCT2_W8[5][3] * dctStep1Com2[3] + offset) >> shift);
            ptrDst[7 * length + currentLine] = (int32_t)((DCT2_W8[7][0] * dctStep1Com2[0] + DCT2_W8[7][1] * dctStep1Com2[1] + DCT2_W8[7][2] * dctStep1Com2[2] + DCT2_W8[7][3] * dctStep1Com2[3] + offset) >> shift);

        }   
    }




    void LLEncoderTrans::LLEncTrans(FrameBuffer& src, FrameBuffer& dst, uint8_t is8, TRANS_TYPE transMode, uint8_t length, uint8_t shift) {
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

    void LLEncoderTrans::LLTrans(FrameBuffer& predResidual, FrameBuffer& transCoeff) {
        uint8_t isWidth8 = (puWidth_ == 4 ? 0 : 1);
        uint8_t isHeight8 = (puHeight_ == 4 ? 0 : 1);
        uint8_t hor_shift = isWidth8 + LL_TRANS1_SHIFT;
        uint8_t ver_shift = isHeight8 + LL_TRANS2_SHIFT;

        int32_t maxLLTransVal = (1 << LL_TRANS_DYNAMIC_BIT) - 1;
        int32_t minLLTransVal = -(1 << LL_TRANS_DYNAMIC_BIT);

        LLEncTrans(predResidual, coeffTransHor_, isWidth8, horTransMode_, puHeight_, hor_shift);
        for (uint8_t i = 0; i < puWidth_*puHeight_; i++) {
            Clip<Pel, Pel>(coeffTransHor_[i], coeffTransHor_[i], minLLTransVal, maxLLTransVal);
        }

        LLEncTrans(coeffTransHor_, transCoeff, isHeight8, verTransMode_, puWidth_, ver_shift);
        for (uint8_t i = 0; i < puWidth_*puHeight_; i++) {
            Clip<Pel, Pel>(transCoeff[i], transCoeff[i], minLLTransVal, maxLLTransVal);
        }
    }
}