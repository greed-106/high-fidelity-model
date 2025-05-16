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
#include "HFTransITrans.h"

namespace ProVivid {
    HFTransITrans::HFTransITrans() {
    }
    
    HFTransITrans::~HFTransITrans() {
    }

    void HFTransITrans::Set(uint8_t colorComponent, uint8_t isITrans) {
        pixNum_ = (colorComponent == Y) ? (MB_SIZE*MB_SIZE) : (MB_SIZE*MB_SIZE >> 1);
        isITrans_ = isITrans;
    }

    void HFTransITrans::ComHFTransITrans(FrameBuffer& src, FrameBuffer& dst) {
        auto ptrSrc = &src[0];
        Pel tmp[MB_SIZE*MB_SIZE];
        for (uint32_t blockNum = 0; blockNum < (pixNum_ >> 2); blockNum++) {
            tmp[blockNum * 4 + 0] = ptrSrc[0] + ptrSrc[1] + ptrSrc[2] + ptrSrc[3];
            tmp[blockNum * 4 + 1] = ptrSrc[0] - ptrSrc[1] + ptrSrc[2] - ptrSrc[3];
            tmp[blockNum * 4 + 2] = ptrSrc[0] + ptrSrc[1] - ptrSrc[2] - ptrSrc[3];
            tmp[blockNum * 4 + 3] = ptrSrc[0] - ptrSrc[1] - ptrSrc[2] + ptrSrc[3];
            ptrSrc += 4;
        }
        auto ptrDst = &dst[0];
        for (uint32_t i = 0; i < pixNum_; i++) {
            if (tmp[i] < 0) {
                ptrDst[i] = (tmp[i] >> 1);
            } else {
                ptrDst[i] = ((tmp[i] + 1) >> 1);
            }
        }
        if (isITrans_) {
            int32_t maxHFITransVal = (1 << HF_ITRANS_DYNAMIC_BIT) - 1;
            int32_t minHFITransVal = -(1 << HF_ITRANS_DYNAMIC_BIT);
            for (uint32_t i = 0; i < pixNum_; i++) {
                Clip<Pel, Pel>(ptrDst[i], ptrDst[i], minHFITransVal, maxHFITransVal);
            }
        }
    }
}

