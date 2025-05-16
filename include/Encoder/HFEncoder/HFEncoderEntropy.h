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
#ifndef HF_ENCODER_ENTROPY_H
#define HF_ENCODER_ENTROPY_H
extern "C" {
#include "cabac.h"
#include "vlc.h"
#include "biariencode.h"
}

#include <vector>

namespace ProVivid {
    extern int VLCTable0(int value);

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

    class HFEncoderEntropy {
    public:
        HFEncoderEntropy();
        ~HFEncoderEntropy();
        void Set(Bitstream* bitstreamVlcHf, EncodingEnvironment* eeCabacHf, HighBandInfoContexts* highBandCtx);
        void HFEntropyCoeffGroupSet(int isLeftBoundaryMb, uint8_t bandIdx, uint8_t colorComponent, uint8_t transformType);
        void HFEntropyCoeffGroup(std::vector<int32_t>& residual, EncDecBlockGroupParams* bgParams, int hfTransformSkipEnable);
        void HFEntropyCoeffBlock(int blockIdx, int* residual, EncDecBlockParams* blockParams);
        void HFEntropyDeltaQp(int isLeftBoundaryMb, int subpicLumaQp, int &mbQp, int &mb_deltaQP);
        void HFEntropyFlag(BiContextTypePtr pBinCtx, int value);
        inline void WriteVLCTable0(int value);
        inline void WriteVLCTable1(int value);
        inline void WriteVLCTable2(int value);
        inline void WriteVLCTable3(int value);
        void HFEncPattern0001(int* resSubBlock);
        EncDecBlockGroupParams bgParams_;

    private:
        Bitstream *bitstream_;
        EncodingEnvironmentPtr eep_;
        HighBandInfoContexts* highBandCtx_;

        int refQp_;
        //uint32_t bits_;

    };
}

#endif // HF_ENCODER_ENTROPY_H
