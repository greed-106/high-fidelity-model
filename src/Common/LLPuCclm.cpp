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
#include "LLPuCclm.h"

namespace ProVivid {
    LLPuCclm::LLPuCclm() {
    }
    
    LLPuCclm::~LLPuCclm() {
    }

    void LLPuCclm::Set(uint32_t mbX, uint32_t mbY, uint8_t colorComponent) {
        leftAvail_ = mbX != 0;
        upAvail_ = mbY != 0;
        colorComponent_ = colorComponent;
    }

    void LLPuCclm::PushNbr(SharedFrameBuffer llBandPicYRec, SharedFrameBuffer llBandPicURec, SharedFrameBuffer llBandPicVRec, uint32_t puYPixelIndex1, uint32_t puYPixelIndex2,bool avg, uint32_t puUVPixelIndex) {
        CclmNbr curP;
        if (avg) {
            curP.y = (llBandPicYRec->at(puYPixelIndex1) + llBandPicYRec->at(puYPixelIndex2)) >> 1;
        } else {
            curP.y = llBandPicYRec->at(puYPixelIndex1);
        }
        curP.u = llBandPicURec->at(puUVPixelIndex);
        curP.v = llBandPicVRec->at(puUVPixelIndex);
        CclmNbr_.push_back(curP);
    }

    void LLPuCclm::GetCclmScale(SharedFrameBuffer llBandPicYRec, SharedFrameBuffer llBandPicURec, SharedFrameBuffer llBandPicVRec, uint32_t puYPixelIndex, uint32_t lineYWidth, uint32_t puUVPixelIndex, uint32_t lineUVWidth) {
        if ((!leftAvail_) && (!upAvail_)) return;
        CclmNbr curP;
        if (leftAvail_ && upAvail_) {
            curP.y = (llBandPicYRec->at(puYPixelIndex - 1) + llBandPicYRec->at(puYPixelIndex - lineYWidth)) >> 1;
            curP.u = (llBandPicURec->at(puUVPixelIndex - 1) + llBandPicURec->at(puUVPixelIndex - lineUVWidth)) >> 1;
            curP.v = (llBandPicVRec->at(puUVPixelIndex - 1) + llBandPicVRec->at(puUVPixelIndex - lineUVWidth)) >> 1;
            CclmNbr_.push_back(curP);
            PushNbr(llBandPicYRec, llBandPicURec, llBandPicVRec, puYPixelIndex - lineYWidth + 6, puYPixelIndex - lineYWidth + 7, true, puUVPixelIndex - lineUVWidth + 3);
            PushNbr(llBandPicYRec, llBandPicURec, llBandPicVRec, puYPixelIndex - 1 + 7 * lineYWidth, 0, false, puUVPixelIndex - 1 + 7 * lineUVWidth);
        } else if (leftAvail_) {
            PushNbr(llBandPicYRec, llBandPicURec, llBandPicVRec, puYPixelIndex - 1, 0, false, puUVPixelIndex - 1);    
            PushNbr(llBandPicYRec, llBandPicURec, llBandPicVRec, puYPixelIndex - 1 + 4 * lineYWidth, 0, false, puUVPixelIndex - 1 + 4 * lineUVWidth);
            PushNbr(llBandPicYRec, llBandPicURec, llBandPicVRec, puYPixelIndex - 1 + 7 * lineYWidth, 0, false, puUVPixelIndex - 1 + 7 * lineUVWidth);

        } else {
            PushNbr(llBandPicYRec, llBandPicURec, llBandPicVRec, puYPixelIndex - lineYWidth, puYPixelIndex - lineYWidth + 1, true, puUVPixelIndex - lineUVWidth);
            PushNbr(llBandPicYRec, llBandPicURec, llBandPicVRec, puYPixelIndex - lineYWidth + 4, puYPixelIndex - lineYWidth + 5, true, puUVPixelIndex - lineUVWidth+2);
            PushNbr(llBandPicYRec, llBandPicURec, llBandPicVRec, puYPixelIndex - lineYWidth + 6, puYPixelIndex - lineYWidth + 7, true, puUVPixelIndex - lineUVWidth + 3);
        }

        std::sort(CclmNbr_.begin(), CclmNbr_.end());
        if (CclmNbr_[1].y * 2 > CclmNbr_[0].y + CclmNbr_[2].y) {
            diffY_ = 2 * CclmNbr_[2].y + CclmNbr_[1].y - 3 * CclmNbr_[0].y;
            diffU_ = 2 * CclmNbr_[2].u + CclmNbr_[1].u - 3 * CclmNbr_[0].u;
            diffV_ = 2 * CclmNbr_[2].v + CclmNbr_[1].v - 3 * CclmNbr_[0].v;
            keyY_ = (CclmNbr_[1].y + CclmNbr_[2].y) >> 1;
            keyUV_[0] = (CclmNbr_[1].u + CclmNbr_[2].u) >> 1;
            keyUV_[1] = (CclmNbr_[1].v + CclmNbr_[2].v) >> 1;
        } else {
            diffY_ = 3 * CclmNbr_[2].y - CclmNbr_[1].y - 2 * CclmNbr_[0].y;
            diffU_ = 3 * CclmNbr_[2].u - CclmNbr_[1].u - 2 * CclmNbr_[0].u;
            diffV_ = 3 * CclmNbr_[2].v - CclmNbr_[1].v - 2 * CclmNbr_[0].v;
            keyY_ = (CclmNbr_[1].y + CclmNbr_[0].y) >> 1;
            keyUV_[0] = (CclmNbr_[1].u + CclmNbr_[0].u) >> 1;
            keyUV_[1] = (CclmNbr_[1].v + CclmNbr_[0].v) >> 1;
        }
        CclmNbr_.clear();

        shift1_ = CCLM_SHIFT1;
        uint32_t shift2 = CCLM_SHIFT2;
        int32_t maxDiffVal = ((1008) << (shift1_ - 10)) - 1;
        int32_t minDiffVal = 0;
        Clip<int32_t, int32_t>(diffY_, diffY_, minDiffVal, maxDiffVal);
        maxDiffVal = (1 << MAX_CCLM_DIFF_UV_BIT) - 1;
        minDiffVal = -(1 << MAX_CCLM_DIFF_UV_BIT);
        Clip<int32_t, int32_t>(diffU_, diffU_, minDiffVal, maxDiffVal);
        Clip<int32_t, int32_t>(diffV_, diffV_, minDiffVal, maxDiffVal);

        int32_t disScale = diffY_ >> (shift1_ - 10);
        int32_t table = 0;
        int32_t sum = 32;
        while (disScale >= sum - 16) {
            table++;
            sum *= 2;
        }
        int32_t table_scale = 0;
        disScale += 16;
        disScale >>= (table);
        disScale &= 15;
        table_scale = CCLM_SCALE[disScale];
        shift1_ += table;
        slopeScale_[0] = (diffU_* table_scale) >> shift2;
        slopeScale_[1] = (diffV_* table_scale) >> shift2;
        shift1_-= shift2;
    }

    void LLPuCclm::ComLLCclmPred(SharedFrameBuffer llBandPicYRec, uint32_t puYPixelIndex, uint32_t lineYWidth, std::vector<int32_t>& intraPred) {
        uint8_t puWidth = 4, puHeight = 8;
        if ((!leftAvail_) && (!upAvail_)) {
            for (uint8_t index = 0; index < puWidth*puHeight; index++) {
                intraPred[index] = INTRA_DEFAULT;
            }
            return;
        }
        int maxPelVal = (1 << MAX_CCLM_PRED_BIT) - 1;
        int minPelVal = 0;
        auto ptrYRec = &(llBandPicYRec->at(puYPixelIndex));
        for (uint8_t h = 0; h < puHeight; ++h) {
            for (uint8_t w = 0; w < puWidth; ++w) {
                int32_t yMean = (ptrYRec[2 * w] + ptrYRec[2 * w + 1]) >> 1;
                int32_t tmpPel = (yMean - keyY_) * slopeScale_[colorComponent_-1];
                tmpPel >>= (shift1_);
                tmpPel += keyUV_[colorComponent_ - 1];
                Clip<int32_t,int32_t>(tmpPel, tmpPel, minPelVal, maxPelVal);
                intraPred[h*puWidth+w] = tmpPel;
            }
            ptrYRec += lineYWidth;
        }
    };

}

