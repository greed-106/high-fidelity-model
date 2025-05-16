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
#ifndef LL_PU_INTER_H
#define LL_PU_INTER_H

#include <fstream>
#include <memory>
#include <unordered_map>
#include "FrameBuffer.h"
#include "BasicTypes.h"
#include "Const.h"

namespace ProVivid {
    class LLPuInter {
    public:
        LLPuInter();
        ~LLPuInter();
        void GeRefWindow(SharedFrameBuffer llBandPicRec_, uint32_t llBandWidth_, uint32_t compId, uint32_t mbY, uint32_t mbX, bool rowFirstFlag, bool rowLastFlag, bool colFirstFlag, bool colLastFlag);
        void GetRefPixel(Pel* src, Pel* dst, uint32_t srcStride, uint32_t dstStride, uint32_t height, uint32_t width);
        void InterPred(MotionVector puMv, std::vector<int32_t>& interPred, uint32_t compId);
        void InterPredInt(std::vector<int32_t>& pred);
        void InterPredHor(std::vector<int32_t>& pred);
        void InterPredVer(std::vector<int32_t>& pred);
        void InterPredHorVer(std::vector<int32_t>& pred);
        void Set(uint32_t compId, uint32_t puWidth, uint32_t puHeight);
    protected:
        SharedPixelBuffer refWinYUV_[N_COLOR_COMP];
        SharedPixelBuffer refWin_;
        int32_t puWidth_;
        int32_t puHeight_;
        int32_t refWinWidth_;
        int32_t refWinHeight_;
        int32_t refWinStride_;
        int32_t refWinOrgIdx_;
        int32_t refPadWidth_;
        int32_t refPadHeight_;
        SharedPixelBuffer pred_;
    };
}

#endif // LL_PU_INTER_H
