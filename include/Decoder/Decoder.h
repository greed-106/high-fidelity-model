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
#include "AlphaDecoder.h"
#include "Utils.h"
//#include "SubPic.h"
#include "Timer.h"
//
namespace HFM {
    class Decoder {
    public:
        Decoder() = delete;
        Decoder(uint32_t bitDepth, std::string decFile, std::string decLLFile, std::string decAlphaFile, const std::string& bitstreamFile, uint32_t intraPeriod);
        ~Decoder();
        void ParseSeqPicHeaderInfo(int frameIdx, Bitstream* bitstream);
        void ParseSeqHeaderInfo(Bitstream* bitstream);
        void ParsePicHeaderInfo(Bitstream* bitstream);
        void RenderingInformation(Bitstream* bitstream);
        void ParseSubPicHeaderInfo(Bitstream* bitstream, int subPicIndex, uint32_t alphaFlag);
        void Decode(uint32_t currFrame, long long& totalBit, long long& cabacBit);
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
        std::string decAlphaFile_;
        std::ofstream decFileHandle_;
        std::ofstream decLLFileHandle_;
        std::ofstream decAlphaFileHandle_;
        std::vector<std::pair<uint32_t, uint32_t>> picWHRaw_{};
        std::vector<std::pair<uint32_t, uint32_t>> picWHRawLL_{};
        std::array<ImgBufSize, LUMA_CHROMA> recPicSize_{};
        std::shared_ptr<SubPicDec> subPicDec_;
        SharedBufferStorage decPicBuffer_;
        SharedBufferStorage decPicLLBuffer_;
        SharedBufferStorage decPicAlphaBuffer_;
        SharedBufferStorage refPicLLBuffer_;
        SeqPicHeaderInfo seqPicHeaderInfo_{};
        std::vector<SubpicSyntaxInfo> subpicSyntaxInfo_;
    };
}

#endif // ENCODER_H
