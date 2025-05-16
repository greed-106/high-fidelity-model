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
#ifndef HF_DECODER_ENTROPY_H
#define HF_DECODER_ENTROPY_H
extern "C" {
#include "cabac.h"
#include "vlc.h"
#include "biaridecod.h"
}
#include "FrameBuffer.h"
#include <cstdint>
#include <vector>

namespace ProVivid {

    typedef struct {
        uint8_t bandIdx;
        uint8_t colorComponent;
        int* leftCoefMax;
        uint8_t transformType;
    } EncDecBlockParams;

    typedef struct {
        uint8_t bandIdx;
        uint8_t colorComponent;
        int leftCoefMax[9];
        int curBlockSize;
        uint8_t transformType;
    } EncDecBlockGroupParams;

    class HFDecoderEntropy {
    public:
        HFDecoderEntropy();
        ~HFDecoderEntropy();
        void Set(Bitstream* bitstream, SubpicSyntaxInfo* subpicSyntaxInfo);
        int HFEntropyFlag(BiContextTypePtr pBinCtx);
        void HFEntropyCoeffGroupSet(int isLeftBoundaryMb, uint8_t bandIdx, uint8_t colorComponent);
        void HFEntropyCoeffBlock(int blockIdx, int* residual, EncDecBlockParams* blockParams);
        void HFEntropyCoeffGroup(uint32_t hfTransformSkipEnable, std::vector<int32_t>& residual);
        void HFEntropyDecode(uint32_t qpDeltaEnable, uint32_t hfTransformSkipEnable, std::vector<int32_t>& residual);
        inline int ReadVLCTable0();
        inline int ReadVLCTable1();
        inline int ReadVLCTable2();
        inline int ReadVLCTable3();
        inline void HFDecPattern0001(int* resSubBlock, int patternIdx);
        
        EncDecBlockGroupParams bgParams_;
        //FrameBuffer residual_;
        uint32_t cbf_;
        int32_t qpDelta;
    private:
        Bitstream bitstreamCabacHf_;
        Bitstream bitstreamVlcHf_;
        DecodingEnvironment de_;
        HighBandInfoContexts highBandCtx_;
        unsigned char blocksNotAllZero[4];
    };
}

#endif // HF_ENCODER_ENTROPY_H
