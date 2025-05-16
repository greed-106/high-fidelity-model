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
#include "LLPuRecon.h"

namespace ProVivid {
    LLPuRecon::LLPuRecon(std::shared_ptr<LLPuITrans> LLPUITrans, std::shared_ptr<IQuant> IQuant) {
        LLPuITrans_ = LLPUITrans;
        IQuant_ = IQuant;
    }
    
    LLPuRecon::~LLPuRecon() {
    }

    void LLPuRecon::Set(uint8_t puWidth, uint8_t puHeight, uint32_t puPixelIndex, uint32_t lineWidth) {
        puWidth_ = puWidth;
        puHeight_ = puHeight;
        puPixelIndex_ = puPixelIndex;
        lineWidth_ = lineWidth;
    }

    void LLPuRecon::ComLLRecon(FrameBuffer& coeff, FrameBuffer& pred, SharedFrameBuffer llBandPicRec, uint8_t qp, uint8_t predMode) {
        
        IQuant_->Set(puWidth_, puHeight_, qp, LL_IQUANT_DYNAMIC_BIT);
        IQuant_->ComIQuant(coeff,coeff, (puWidth_ >> 3) + (puHeight_ >> 3));

        LLPuITrans_->Set(puWidth_, puHeight_, predMode);
        LLPuITrans_->ComLLITrans(coeff);

        auto ptrRec = &(llBandPicRec->at(puPixelIndex_));
        uint32_t maxRecVal = (1 << LL_REC_DYNAMIC_BIT) - 1;
        for (uint8_t j = 0; j < puHeight_; ++j) {
            for (uint8_t i = 0; i < puWidth_; i++) {
                ptrRec[i] = coeff[j * puWidth_ + i] + pred[j * puWidth_ + i];
                Clip<int32_t, int32_t>((ptrRec[i]), (ptrRec[i]), 0, maxRecVal);
            }
            ptrRec += lineWidth_;
        }

    }


    void LLPuRecon::ComLLRecon(FrameBuffer& pred, SharedFrameBuffer llBandPicRec) {
        auto ptrRec = &(llBandPicRec->at(puPixelIndex_));
        for (uint8_t j = 0; j < puHeight_; ++j) {
            for (uint8_t i = 0; i < puWidth_; i++) {
                ptrRec[i] = pred[j * puWidth_ + i];
            }
            ptrRec += lineWidth_;
        }
    }
}

