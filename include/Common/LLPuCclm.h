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
#ifndef LL_PU_CCLM_H
#define LL_PU_CCLM_H

#include "FrameBuffer.h"
#include "queue"
#include "Table.h"
#include "Const.h"
#include "Utils.h"
#include "algorithm"

namespace HFM {

    class LLPuCclm {
    public:
        LLPuCclm();
        ~LLPuCclm();
        void Set(uint32_t mbX, uint32_t mbY, uint8_t colorComponent);
        void PushNbr(SharedFrameBuffer llBandPicYRec, SharedFrameBuffer llBandPicURec, SharedFrameBuffer llBandPicVRec, uint32_t puYPixelIndex1, uint32_t puYPixelIndex2, bool avg, uint32_t puUVPixelIndex);
        void GetCclmScale(SharedFrameBuffer llBandPicYRec, SharedFrameBuffer llBandPicURec, SharedFrameBuffer llBandPicVRec, uint32_t puYPixelIndex, uint32_t lineYWidth, uint32_t puUVPixelIndex, uint32_t lineUVWidth);
        void ComLLCclmPred(SharedFrameBuffer llBandPicYRec, uint32_t puYPixelIndex, uint32_t lineYWidth, std::vector<int32_t>& intraPred);
    private:
        uint8_t colorComponent_;
        uint32_t shift1_;
        bool leftAvail_, upAvail_;
        int32_t diffY_,diffU_,diffV_;
        int32_t keyY_, keyUV_[2];
        int32_t slopeScale_[2];
        struct CclmNbr {
            Pel y, u, v;
            bool operator<(const CclmNbr &t) const {
                return (y < t.y);
            }
        };
        std::vector<CclmNbr> CclmNbr_;
    };
}

#endif // LL_PU_INTRA_H
