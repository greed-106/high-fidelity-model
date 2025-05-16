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

namespace ProVivid {
    class LLDecoderEntropy {
    public:
        LLDecoderEntropy(Bitstream* bitstream, SubpicSyntaxInfo* subpicSyntaxInfo);
        ~LLDecoderEntropy();
        void LLEntropyCoeff(int component, std::vector<int32_t>& residual);
        void LLEntropyMbInfo(uint32_t frameType, uint32_t qpDeltaEnable);
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
