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

