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
#ifndef WAVELET_H
#define WAVELET_H

#include "BasicTypes.h"
#include "FrameBuffer.h"

namespace ProVivid {
    constexpr uint32_t DWT_SHIFT = 2;
    constexpr uint32_t DWT97_FILTER_LEN_LO = 9;
    constexpr uint32_t DWT97_FILTER_LEN_HI = 7;
    constexpr uint32_t IDWT97_FILTER_LEN_LO_EVEN = 1;
    constexpr uint32_t IDWT97_FILTER_LEN_HI_EVEN = 2;
    constexpr uint32_t IDWT97_FILTER_LEN_LO_ODD = 4;
    constexpr uint32_t IDWT97_FILTER_LEN_HI_ODD = 5;
    constexpr int32_t DWT_LL_SUB_BAND_CLIP_MAX_SCALE = 2;
    constexpr int32_t DWT_LL_SUB_BAND_ADD_SCALE = 1;
    constexpr int32_t DWT_LL_SUB_BAND_ADD_SHIFT = 1;
    constexpr Pel LL_SUB_BAND_MIN = 0;
    constexpr Pel LL_SUB_BAND_MAX = DWT_LL_SUB_BAND_CLIP_MAX_SCALE * (1 << (10 + DWT_SHIFT)) - 1;
    constexpr Pel LL_SUB_BAND_ADD = DWT_LL_SUB_BAND_ADD_SCALE * (1 << (10 + DWT_SHIFT - DWT_LL_SUB_BAND_ADD_SHIFT));
    constexpr uint32_t IDWT_CLIP_BIT_DEPTH = 15;
    constexpr int32_t IDWT_CLIP_MIN = (-(1 << (IDWT_CLIP_BIT_DEPTH - 1)));
    constexpr int32_t IDWT_CLIP_MAX = ((1 << (IDWT_CLIP_BIT_DEPTH - 1)) - 1);

    typedef enum Idwt97FilterMode {
        IDWT97_FILTER_LO_EVEN,
        IDWT97_FILTER_HI_EVEN,
        IDWT97_FILTER_LO_ODD,
        IDWT97_FILTER_HI_ODD
    } Idwt97FilterMode;

    bool DWT53(const Pel* input, int len, Pel* subBandLF, Pel* subBandHF, int strideIn = 1, int strideOut = 1);
    bool IDWT53(const Pel* subBandLF, const Pel* subBandHF, Pel* rec, int len, int strideSubBand = 1, int strideRec = 1);
    void SetConvInput(const Pel* input, int len, int currPos, int filterLen, Pel* output);
    bool IntConvDWT97(const Pel* input, int n, Pel* subBandLF, Pel* subBandHF);
    void SetDeconvInput(const Pel* input, int len, int currPos, int filterLen, Idwt97FilterMode mode, Pel* output);
    bool IntDeconvDWT97(const Pel* subBandLF, const Pel* subBandHF, int n, Pel* output);
    void IDWT(SubPicInfoMap& subPicInfo, SubBandMap& subBands, const SharedFrameBuffer& tmpFrameBuffer, const SharedFrameBuffer& rowBuffer, std::vector<std::pair<uint32_t, uint32_t>> recSubPicSize, uint32_t bitDepth);
}

#endif // WAVELET_H
