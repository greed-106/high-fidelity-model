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

namespace ProVivid {
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
