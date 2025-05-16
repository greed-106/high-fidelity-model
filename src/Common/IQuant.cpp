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
#include "IQuant.h"

namespace ProVivid {
    IQuant::IQuant() {
    }
    
    IQuant::~IQuant() {
    }

    void IQuant::Set(uint8_t puWidth, uint8_t  puHeight, uint8_t qp, uint32_t iQuantRangeBit) {
        puWidth_ = puWidth;
        puHeight_ = puHeight;
        qp_ = qp;
        maxIQuantVal_ = (1 << iQuantRangeBit) - 1;
        minIQuantVal_ = -(1 << iQuantRangeBit);
    }

    void IQuant::ComIQuant(FrameBuffer& src, FrameBuffer& dst, uint8_t sizeIndex) {
        uint32_t scale = DEQUANT_SCALE[sizeIndex][(qp_ + 12) % 8];
        int8_t iQShift = DEQUANT_SHIFT - (qp_ + 12) / 8;
        int32_t offset = (iQShift <= 0) ? 0 : (1 << (iQShift - 1));

        auto ptrSrc = &src[0];
        auto ptrDst = &dst[0];
        for (uint8_t i = 0; i < puHeight_; i++) {
            for (uint8_t j = 0; j < puWidth_; j++) {
                int32_t coeffTmp = *ptrSrc;
                if (iQShift < 0) {
                    coeffTmp = (coeffTmp*(int32_t)scale) << (-iQShift);
                } else {
                    coeffTmp = (coeffTmp*(int32_t)scale+offset) >> (iQShift);
                }
                Clip<int32_t, int32_t>(coeffTmp, coeffTmp, minIQuantVal_, maxIQuantVal_);
                (*ptrDst) = coeffTmp;
                ptrSrc++;
                ptrDst++;
            }
        }
    }
}

