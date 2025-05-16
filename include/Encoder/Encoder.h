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
#ifndef ENCODER_H
#define ENCODER_H

#include <array>
#include <vector>
#include "SubPicEnc.h"
extern "C" {
#include "vlc.h" 
#include "cabac.h"
#include "biariencode.h"
#include "context_ini.h"
}
#include "HFEncoder.h"
#include "LLEncoder.h"
#include "SubPic.h"
#include "Timer.h"

namespace HFM {
    class Encoder {
    public:
        Encoder() = delete;
        Encoder(std::string recFile, std::string recLLFile, uint32_t frameCount, uint32_t bitDepth, uint32_t intraPeriod);
        ~Encoder();
        void SetInput(std::shared_ptr<SubPicEnc> subPic, 
            int32_t qp, int32_t cbQpOffset, int32_t crQpOffset, int32_t hlQpOffset, int32_t lhQpOffset, int32_t hhQpOffset,
            bool qpDeltaEnable, bool hfTransformSkip);
        void Encode(uint32_t currFrame);
        void StreamCabacCtxIni();
        void SubpicEncodingDone(uint32_t subpicIndex);
        void PicEncodingDone();
        void DetermineMBdeltaQP(uint32_t bandWidth, uint32_t bandHeight);
        Bitstream bitstream_;
        int seqHeaderBytes_;
    private:
        std::string recFile_;
        std::string recLLFile_;
        uint32_t frameCount_;
        uint32_t bitDepth_;
        uint32_t intraPeriod_;
        uint8_t qp_;
        int32_t cbQpOffset_, crQpOffset_, hlQpOffset_, lhQpOffset_, hhQpOffset_;
        bool qpDeltaEnable_, hfTransformSkip_;
        std::ofstream recFileHandle_;
        std::ofstream recLLFileHandle_;
        std::shared_ptr<SubPicEnc> subPic_;
        std::vector<SubPicInfoMap> subPicInfoRec_;
        std::vector<SubPicInfoMap> subPicLLInfoRec_;
        std::vector<SubPicInfoMap> subPicLLInfoRef_;
        SharedBufferStorage recPicBuffer_;
        SharedBufferStorage recPicLLBuffer_;
        std::vector<std::pair<uint32_t, uint32_t>> picWHRaw_{};
        std::vector<std::pair<uint32_t, uint32_t>> picWHRawLL_{};
        std::array<ImgBufSize, LUMA_CHROMA> recPicSize_{};
        SharedBufferStorage refPicLLBuffer_;
        Bitstream bitstreamCabacLl_;
        Bitstream bitstreamVlcLl_;
        Bitstream bitstreamCabacHf_;
        Bitstream bitstreamVlcHf_;
        EncodingEnvironment eeCabacLl_;
        EncodingEnvironment eeCabacHf_;
        TextureInfoContexts texCtx_;
        MotionInfoContexts  motCtx_;
        HighBandInfoContexts highBandCtx_;
        std::vector<std::vector<int16_t>> mbDeltaQP_;
    };
}

#endif // ENCODER_H
