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
#ifndef DECODER_H
#define DECODER_H

#include "SubPicDec.h"
extern "C" {
#include "vlc.h" 
#include "Entropy.h"
//#include "cabac.h"
//#include "biariencode.h"
//#include "context_ini.h"
}
#include "HFDecoder.h"
#include "LLDecoder.h"
#include "Utils.h"
//#include "SubPic.h"
#include "Timer.h"
//
namespace ProVivid {
    class Decoder {
    public:
        Decoder() = delete;
        Decoder(uint32_t bitDepth, std::string decFile, std::string decLLFile, const std::string& bitstreamFile, uint32_t intraPeriod);
        ~Decoder();
        void ParseSeqPicHeaderInfo(int frameIdx, Bitstream* bitstream);
        void ParseSeqHeaderInfo(Bitstream* bitstream);
        void ParsePicHeaderInfo(Bitstream* bitstream);
        void RenderingInformation(Bitstream* bitstream);
        void Decode(uint32_t currFrame);
        void SetSubPic(std::shared_ptr<SubPicDec> subPicDec);
        Bitstream bitstream_;
        int64_t bitstreamLength_;
        int64_t curBitstreamPos_;
        std::vector<SubPicInfoMap> subPicInfoDec_;
        std::vector<SubPicInfoMap> subPicLLInfoDec_;
        std::vector<SubPicInfoMap> subPicLLInfoRef_;
    private:
        uint32_t qp_;
        uint32_t intraPeriod_;
        uint32_t bitDepth_{10};
        std::string decFile_;
        std::string decLLFile_;
        std::ofstream decFileHandle_;
        std::ofstream decLLFileHandle_;
        std::vector<std::pair<uint32_t, uint32_t>> picWHRaw_{};
        std::vector<std::pair<uint32_t, uint32_t>> picWHRawLL_{};
        std::array<ImgBufSize, LUMA_CHROMA> recPicSize_{};
        std::shared_ptr<SubPicDec> subPicDec_;
        SharedBufferStorage decPicBuffer_;
        SharedBufferStorage decPicLLBuffer_;
        SharedBufferStorage refPicLLBuffer_;
        SeqPicHeaderInfo seqPicHeaderInfo_{};
        std::vector<SubpicSyntaxInfo> subpicSyntaxInfo_;
    };
}

#endif // ENCODER_H
