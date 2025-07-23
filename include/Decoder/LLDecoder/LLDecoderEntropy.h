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
#ifndef LL_DECODER_ENTROPY_H
#define LL_DECODER_ENTROPY_H

#include <cstring>
#include "FrameBuffer.h"
extern "C" {
#include "cabac.h"
#include "vlc.h"
#include "biaridecod.h"
}

#include <vector>

namespace HFM {
    class LLDecoderEntropy {
    public:
        LLDecoderEntropy(Bitstream* bitstream, SubpicSyntaxInfo* subpicSyntaxInfo);
        ~LLDecoderEntropy();
        void LLEntropyCoeff(PixelFormat pixelFormat, int component, std::vector<int32_t>& residual);
        void LLEntropyMbInfo(uint32_t frameType, uint32_t qpDeltaEnable, uint32_t cclmEnable);
        void LLEntropyDone();
        MbSyntaxInfo mbEntropyInfo_;

    private:
        Bitstream bitstreamCabacLl_;
        Bitstream bitstreamVlcLl_;
        DecodingEnvironment de_;
        TextureInfoContexts texCtx_;
        MotionInfoContexts motCtx_;
    };
}

#endif // LL_DECODER_ENTROPY_H
