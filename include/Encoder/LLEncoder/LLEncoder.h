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
#ifndef LL_ENCODER_H
#define LL_ENCODER_H

#include "FrameBuffer.h"
#include "LLEncoderTrans.h"
#include "EncoderQuant.h"
#include "LLPuIntra.h"
#include "LLPuInter.h"
#include "LLEncoderME.h"
#include "LLPuCclm.h"
#include "LLPuRecon.h"
#include "LLEncoderEntropy.h"
#include "BasicTypes.h"
#include "Const.h"
#include "LLEncoderME.h"
#include "Utils.h"
#include "Tool.h"

namespace HFM {
    class LLEncoder {
    public:
        LLEncoder();
        ~LLEncoder();
        void Set(uint32_t llBandWidth_, uint32_t llBandHeight_, std::vector<SharedFrameBuffer>& llBandPic, std::vector<SharedFrameBuffer>& llBandPicRec, uint8_t qp_, int32_t cbQpOffset, int32_t crQpOffset);
        void Set(uint32_t llBandWidth_, uint32_t llBandHeight_, std::vector<SharedFrameBuffer>& llBandPic, std::vector<SharedFrameBuffer>& llBandPicRec, std::vector<SharedFrameBuffer>& llBandPicRef, uint8_t qp_, int32_t cbQpOffset, int32_t crQpOffset);
        void GetPuWH(uint8_t tuSize);
        void LLEncode(Bitstream *bitstreamVlcLl, EncodingEnvironment *eeCabacLl, TextureInfoContexts *texCtx, MotionInfoContexts *motCtx, int frameType, bool qpDelta, std::vector<std::vector<int16_t>>& mbDeltaQP);
        void LLEncodePu(uint8_t tuSize, bool interPredFlag_, uint8_t intraPredMode, bool withResi = true);
        void LLEncodeIntraPu(uint8_t tuSize, uint8_t intraPredMode);
        void LLEncodeInterPu(MotionVector mv, bool withResi);
        void Residual();
        uint32_t Distortion();
        void IntraRdCost();
        void InterRdCost(float curRdCost);
        void LLEncode();
        void NextMB();
    private:
        std::shared_ptr<LLPuIntra> llPuIntra_;
        std::shared_ptr<LLEncoderME> llPuInter_;
        std::shared_ptr<LLPuCclm> llPuCclm_;
        std::shared_ptr<LLEncoderTrans>llEncoderTrans_;
        std::shared_ptr<EncoderQuant>encoderQuant_;
        std::shared_ptr<LLPuITrans>llPuITrans_;
        std::shared_ptr<IQuant>llIQuant_;
        std::shared_ptr<LLPuRecon>llPuRecon_;
        std::shared_ptr<LLEncoderEntropy>llEncoderEntropy_;

        std::vector<SharedFrameBuffer>* llBandPic_;
        std::vector<SharedFrameBuffer>* llBandPicRef_;
        std::vector<SharedFrameBuffer>* llBandPicRec_;
        //std::unordered_map<ColorComp, SubPicInfo>& subPicInfo_;
        uint32_t llBandWidth_;
        uint32_t llBandHeight_;
        uint8_t qp_[N_COLOR_COMP];
        uint8_t mbQp_[N_COLOR_COMP];
        uint32_t mbX_, mbY_;
        uint32_t puPixelIndex_;
        FrameBuffer puPred_;
        FrameBuffer predResidual_;
        FrameBuffer coeff_;
        SharedFrameBuffer pBestYRec_;
        uint8_t puWidth_, puHeight_;
        uint8_t puSize_;
        uint8_t isChroma_;
        uint8_t colorComponent_;
        uint32_t lineWidth_;

        uint32_t rate_, dis_;

        //intra
        uint8_t puX_, puY_;
        uint8_t intraPredMode_;
        float bestIntraRdcost_[LUMA_CHROMA];
        uint8_t bestLumaPuSize_;
        uint8_t bestIntraPredMode_[LUMA_CHROMA];

        //inter
        bool interPredFlag_;
        bool bestMbInter_;
        float bestInterRdcost_;
        MotionVector mv_;
        uint32_t MVDInfo_;
        uint32_t resInfo_;
        uint32_t bestMVDInfo_;
        uint32_t bestResInfo_;
        MotionVector bestMv_;
        MotionVector preMv_;

    };
}

#endif // LL_ENCODER_H
