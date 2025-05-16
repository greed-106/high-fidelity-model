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
#ifndef HF_DECODER_H
#define HF_DECODER_H

#include <vector>
#include "FrameBuffer.h"
#include "Const.h"
#include "Table.h"
#include "HFDecoderEntropy.h"
#include "IQuant.h"
#include "HFTransITrans.h"


namespace ProVivid {
    class HFDecoder {
    public:
        HFDecoder();
        ~HFDecoder();
        void Set(uint32_t hfBandWidth, uint32_t hfBandHeight, SubBandMap& subPicDec,
            uint8_t qp, int32_t cbQpOffset, int32_t crQpOffset, int32_t hlQpOffset, int32_t lhQpOffset, int32_t hhQpOffset);
        void HFDecode(Bitstream* bitstream, SeqPicHeaderInfo* seqPicHeaderInfo, SubpicSyntaxInfo* subpicSyntaxInfo);
        void hfMBDecReorder();

    private:
        std::shared_ptr<IQuant>hfIQuant_;
        std::shared_ptr<HFTransITrans>hfTransITrans_;
        std::shared_ptr<HFDecoderEntropy>hfDecoderEntropy_;

        SubBandMap* subPicDec_;
        uint8_t qp_[N_SUB_BANDS][N_COLOR_COMP];
        uint32_t hfBandWidth_, hfBandHeight_;
        uint8_t colorComponent_;
        uint8_t componentShiftX_;  
        uint8_t hfBandIdx_;
        uint32_t pixelIndex_;
        uint32_t lineWidth_;
        FrameBuffer mbCoeff_;
        FrameBuffer mbDec_;

    };
}

#endif // LL_ENCODER_H
