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
#ifndef LL_DECODER_H
#define LL_DECODER_H

#include "LLDecoderEntropy.h"
#include "BasicTypes.h"
#include "Entropy.h"
#include "Const.h"
#include "LLPuIntra.h"
#include "LLPuInter.h"
#include "LLPuCclm.h"
#include "LLPuRecon.h"


namespace HFM {
    class LLDecoder {
    public:
        LLDecoder();
        ~LLDecoder();
        void Set(uint32_t llBandWidth_, uint32_t llBandHeight_, std::vector<SharedFrameBuffer>& llBandPic,
            uint8_t qp_, int32_t cbQpOffset, int32_t crQpOffset);
        void Set(uint32_t llBandWidth_, uint32_t llBandHeight_, std::vector<SharedFrameBuffer>& llBandPic, std::vector<SharedFrameBuffer>& llBandPicRef,
            uint8_t qp_, int32_t cbQpOffset, int32_t crQpOffset);
        void GetPuWH(uint8_t tuSize);
        void LLDecode(Bitstream* bitstream, SeqPicHeaderInfo* seqPicHeaderInfo, SubpicSyntaxInfo* subpicSyntaxInfo);

    private:
        std::shared_ptr<LLPuIntra> llPuIntra_;
        std::shared_ptr<LLPuInter> llPuInter_;
        std::shared_ptr<LLPuCclm> llPuCclm_;
        std::shared_ptr<LLPuITrans>llPuITrans_;
        std::shared_ptr<IQuant>llIQuant_;
        std::shared_ptr<LLPuRecon>llPuRecon_;

        PixelFormat pixelFormat_;
        uint32_t puPixelIndex_;
        FrameBuffer puPred_;
        uint32_t llBandWidth_;
        uint32_t llBandHeight_;
        uint8_t qp_[N_COLOR_COMP];
        uint8_t mbQp_[N_COLOR_COMP];
        std::vector<SharedFrameBuffer>* llBandPic_;
        std::vector<SharedFrameBuffer>* llBandPicRef_;
        FrameBuffer coeff_;
        uint8_t puWidth_, puHeight_;
        uint8_t isChroma_;
        uint8_t colorComponent_;
        uint32_t lineWidth_;

        //intra
        uint8_t puX_, puY_;

    };
}

#endif // LL_ENCODER_H
