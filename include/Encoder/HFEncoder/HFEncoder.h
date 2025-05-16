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
#ifndef HF_ENCODER_H
#define HF_ENCODER_H

#include <vector>
#include "FrameBuffer.h"
#include "Const.h"
#include "Table.h"
#include "EncoderQuant.h"
#include "HFEncoderEntropy.h"
#include "IQuant.h"
#include "HFTransITrans.h"
#include <cstring>

namespace HFM {
    class HFEncoder {
    public:
        HFEncoder();
        ~HFEncoder();
        void Set(uint32_t hfBandWidth, uint32_t hfBandHeight, SubBandMap& subPic, SubBandMap& subPicRec,
            uint8_t qp, int32_t cbQpOffset, int32_t crQpOffset, int32_t hlQpOffset, int32_t lhQpOffset, int32_t hhQpOffset);
        uint32_t SimpleRd(uint8_t qp, FrameBuffer &mbPix);
        void HFEncode(Bitstream* bitstreamVlcHf_, EncodingEnvironment* eeCabacHf_, HighBandInfoContexts* highBandCtx_, bool qpDelta, std::vector<std::vector<int16_t>>& mbDeltaQP, bool hfTransformSkip);
        void hfMBOriReorder();
        void hfMBRecReorder(uint8_t bestTransType);
        void ChromaEnhanceQuant(FrameBuffer& mbPix);

    private:
        std::shared_ptr<EncoderQuant>encoderQuant_;
        std::shared_ptr<IQuant>hfIQuant_;
        std::shared_ptr<HFTransITrans>hfTransITrans_;
        std::shared_ptr<HFEncoderEntropy>hfEncoderEntropy_;

        SubBandMap* subPic_;
        SubBandMap* subPicRec_;
        uint8_t qp_[N_SUB_BANDS][N_COLOR_COMP];
        uint8_t mbQp_[N_SUB_BANDS][N_COLOR_COMP];
        uint32_t hfBandWidth_, hfBandHeight_;
        uint8_t colorComponent_;
        uint8_t componentShiftX_;
        uint8_t hfBandIdx_;
        uint32_t pixelIndex_;
        uint32_t lineWidth_;
        uint8_t bestTransType_;
        FrameBuffer mbOri_;
        FrameBuffer mbHad_;
        FrameBuffer mbCoeff_;
        FrameBuffer mbRec_;
    };
}

#endif // LL_ENCODER_H
