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
#ifndef LL_ENCODER_ENTROPY_H
#define LL_ENCODER_ENTROPY_H
extern "C" {
#include <cstring>
#include "cabac.h"
#include "vlc.h"
}

#include <vector>
#include "BasicTypes.h"

namespace HFM {
    class LLEncoderEntropy {
    public:
        LLEncoderEntropy();
        ~LLEncoderEntropy();
        void Set(Bitstream* bitstreamVlcLl, EncodingEnvironment* eeCabacLl, TextureInfoContexts* texCtx, MotionInfoContexts* motCtx);
        void SetInter(uint8_t colorComponent);
        void LLEntropyCoeff(PixelFormat pixelFormat, uint8_t colorComponent, uint8_t tuSize, uint8_t predMode, std::vector<int32_t>& residual);
        void LLEntropyPuSize(uint8_t isIntra, uint8_t colorComponent, uint8_t puSize);
        void LLEntropyIntraPredMode(uint8_t colorComponent, uint8_t predModeize, bool cclmEnable);
        void LLEntropyMbMode(uint8_t isPframe, uint8_t mbMode);
        void LLEntropyInterMode(uint8_t interNoResidualFlag);
        void LLEntropyMvdMode(uint8_t mvdFlag);
        void LLEntropyMvd(int mvd, int mvp, int isMvdY, int cond);
        void LLEntropyDeltaQp(int isLeftBoundaryMb, int subpicLumaQp, int &mbQp, int &qp_delta);
        void GetCabcaState();
        void ResetCabcaState();
        uint32_t bits_;
    private:
        Bitstream* bitstream_;
        EncodingEnvironmentPtr eep_;
        TextureInfoContexts* texCtx_;
        MotionInfoContexts* motCtx_;

        EncodingEnvironment eeState_;
        int cabacLenState;
        TextureInfoContexts texCtxState_;
        MotionInfoContexts motCtxState_;

        int refQp_;
    };
}

#endif // LL_ENCODER_ENTROPY_H
