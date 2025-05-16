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


namespace ProVivid {
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
