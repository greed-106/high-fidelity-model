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
