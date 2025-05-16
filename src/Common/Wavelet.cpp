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
#include "BasicTypes.h"
#include "Utils.h"
#include "Wavelet.h"
#include "Tool.h"

namespace ProVivid {
    bool DWT53(const Pel* input, int len, Pel* subBandLF, Pel* subBandHF, int strideIn, int strideOut) {
        int lenLo = (len + 1) >> 1;
        int lenHi = len - lenLo;
        bool isOddLength = (len & 1);

        // split and predict1
        for (int i = 0; i < lenHi - 1; i++) {
            subBandHF[i * strideOut] = input[(2 * i + 1) * strideIn] - ((input[2 * i * strideIn] + input[(2 * i + 2) * strideIn] + 1) >> 1);
        }
        if (isOddLength) {
            subBandHF[(lenHi - 1) * strideOut] = input[(len - 2) * strideIn] - ((input[(len - 3) * strideIn] + input[(len - 1) * strideIn] + 1) >> 1);
        } else {
            subBandHF[(lenHi - 1) * strideOut] = input[(len - 1) * strideIn] - ((2 * input[(len - 2) * strideIn] + 1) >> 1);
        }

        // split and update1
        subBandLF[0] = input[0] + ((2 * subBandHF[0] + 2) >> 2);
        for (int i = 1; i < lenLo - 1; ++i) {
            subBandLF[i * strideOut] = input[(2 * i) * strideIn] + (((subBandHF[i * strideOut] + subBandHF[(i - 1) * strideOut]) + 2) >> 2);
        }
        if (isOddLength) {
            subBandLF[(lenLo - 1) * strideOut] = input[(len - 1) * strideIn] + ((2 * subBandHF[(lenHi - 1) * strideOut] + 2) >> 2);
        } else {
            subBandLF[(lenLo - 1) * strideOut] = input[(len - 2) * strideIn] + (((subBandHF[(lenHi - 2) * strideOut] + subBandHF[(lenHi - 1) * strideOut]) + 2) >> 2);
        }

        // scale
        for (int i = 0; i < lenHi; i++) {
            subBandHF[i * strideOut] >>= 1;
        }
        return true;
    }

    bool IDWT53(const Pel* subBandLF, const Pel* subBandHF, Pel* rec, int len, int strideSubBand, int strideRec) {
        int lenLo = (len + 1) >> 1;
        int lenHi = len - lenLo;
        int isOddLength = (len & 1);

        // inv-update and merge
        if (isOddLength) {
            rec[(len - 1) * strideRec] = subBandLF[(lenLo - 1) * strideSubBand] - ((2 * (subBandHF[(lenHi - 1) * strideSubBand] << 1) + 2) >> 2);
            Clip(rec[(len - 1) * strideRec], rec[(len - 1) * strideRec], IDWT_CLIP_MIN, IDWT_CLIP_MAX);
        } else {
            rec[(len - 2) * strideRec] = subBandLF[(lenLo - 1) * strideSubBand] - (((subBandHF[(lenHi - 2) * strideSubBand] << 1) + (subBandHF[(lenHi - 1) * strideSubBand] << 1) + 2) >> 2);
            Clip(rec[(len - 2) * strideRec], rec[(len - 2) * strideRec], IDWT_CLIP_MIN, IDWT_CLIP_MAX);
        }
        for (int i = 1; i < lenLo - 1; ++i) {
            rec[2 * i * strideRec] = subBandLF[i * strideSubBand] - (((subBandHF[i * strideSubBand] << 1) + (subBandHF[(i - 1) * strideSubBand] << 1) + 2) >> 2);
            Clip(rec[2 * i * strideRec], rec[2 * i * strideRec], IDWT_CLIP_MIN, IDWT_CLIP_MAX);
        }
        rec[0] = subBandLF[0] - ((2 * (subBandHF[0] << 1) + 2) >> 2);
        Clip(rec[0], rec[0], IDWT_CLIP_MIN, IDWT_CLIP_MAX);

        // inv-predict and merge
        if (isOddLength) {
            rec[(len - 2) * strideRec] = (subBandHF[(lenHi - 1) * strideSubBand] << 1) + (((rec[(len - 1) * strideRec] + rec[(len - 3) * strideRec]) + 1) >> 1);
            Clip(rec[(len - 2) * strideRec], rec[(len - 2) * strideRec], IDWT_CLIP_MIN, IDWT_CLIP_MAX);
        } else {
            rec[(len - 1) * strideRec] = (subBandHF[(lenHi - 1) * strideSubBand] << 1) + ((2 * rec[(len - 2) * strideRec] + 1) >> 1);
            Clip(rec[(len - 1) * strideRec], rec[(len - 1) * strideRec], IDWT_CLIP_MIN, IDWT_CLIP_MAX);
        }
        for (int i = 0; i < lenHi - 1; i++) {
            rec[(2 * i + 1) * strideRec] = (subBandHF[i * strideSubBand] << 1) + (((rec[2 * i * strideRec] + rec[(2 * i + 2) * strideRec]) + 1) >> 1);
            Clip(rec[(2 * i + 1) * strideRec], rec[(2 * i + 1) * strideRec], IDWT_CLIP_MIN, IDWT_CLIP_MAX);
        }
        return true;
    }

    void SetConvInput(const Pel* input, int len, int currPos, int filterLen, int32_t* output) {
        for (int i = 0; i < filterLen ; ++i) {
            int id = currPos + i - filterLen / 2;
            if (id < 0) {
                id = -id;
            }
            if (id > len - 1) {
                id = 2 * (len - 1) - id;
            }
            output[i] = input[id];
        }
    }

    bool IntConvDWT97(const Pel* input, int n, Pel* subBandLF, Pel* subBandHF) {
        int32_t loPass[DWT97_FILTER_LEN_LO] = {0};
        int32_t hiPass[DWT97_FILTER_LEN_HI] = {0};
        int idx = 0;
        for (int i = 0; i < n; i = i + 2) {
            SetConvInput(input, n, i, DWT97_FILTER_LEN_LO, loPass);
            SetConvInput(input, n, i + 1, DWT97_FILTER_LEN_HI, hiPass);
            subBandLF[idx] = loPass[0] >> 6;
            subBandLF[idx] -= loPass[2] >> 3;
            subBandLF[idx] += loPass[3] >> 2;
            subBandLF[idx] += loPass[4] >> 1;
            subBandLF[idx] += loPass[4] >> 2;
            subBandLF[idx] -= loPass[4] >> 5;
            subBandLF[idx] += loPass[5] >> 2;
            subBandLF[idx] -= loPass[6] >> 3;
            subBandLF[idx] += loPass[8] >> 6;
            Clip(subBandLF[idx], subBandLF[idx], IDWT_CLIP_MIN, IDWT_CLIP_MAX);

            subBandHF[idx] = hiPass[0] >> 5;
            subBandHF[idx] -= hiPass[2] >> 2;
            subBandHF[idx] -= hiPass[2] >> 5;
            subBandHF[idx] += hiPass[3] >> 1;
            subBandHF[idx] -= hiPass[4] >> 2;
            subBandHF[idx] -= hiPass[4] >> 5;
            subBandHF[idx] += hiPass[6] >> 5;
            Clip(subBandHF[idx], subBandHF[idx], IDWT_CLIP_MIN, IDWT_CLIP_MAX);
            idx++;
        }
        return true;
    }

    void SetDeconvInput(const Pel* input, int len, int currPos, int filterLen, Idwt97FilterMode mode, Pel* output) {
        for (int j = 0; j < filterLen; ++j) {
            int id = currPos + j - filterLen / 2;
            if (mode == IDWT97_FILTER_LO_ODD) {
                id++;
            }
            if (id < 0) {
                id = -id;
                if (mode == IDWT97_FILTER_HI_EVEN || mode == IDWT97_FILTER_HI_ODD) {
                    id--; // symmetric type, high: X|X, low: X|Y|X
                }
            }
            if (id > len - 1) {
                id = 2 * (len - 1) - id;
                if (mode == IDWT97_FILTER_LO_EVEN || mode == IDWT97_FILTER_LO_ODD) {
                    id++; // symmetric type: low: X|X, high: X|Y|X
                }
            }
            output[j] = input[id];
        }
    }

    bool IntDeconvDWT97(const Pel* subBandLF, const Pel* subBandHF, const int n, Pel* output) {
        Pel loEven[IDWT97_FILTER_LEN_LO_EVEN] = {0};
        Pel hiEven[IDWT97_FILTER_LEN_HI_EVEN] = {0};
        Pel loOdd[IDWT97_FILTER_LEN_LO_ODD] = {0};
        Pel hiOdd[IDWT97_FILTER_LEN_HI_ODD] = {0};
        for (int i = 0; i < n; ++i) {
            SetDeconvInput(subBandLF, n, i, IDWT97_FILTER_LEN_LO_EVEN, IDWT97_FILTER_LO_EVEN, loEven);
            SetDeconvInput(subBandHF, n, i, IDWT97_FILTER_LEN_HI_EVEN, IDWT97_FILTER_HI_EVEN, hiEven);
            SetDeconvInput(subBandLF, n, i, IDWT97_FILTER_LEN_LO_ODD, IDWT97_FILTER_LO_ODD, loOdd);
            SetDeconvInput(subBandHF, n, i, IDWT97_FILTER_LEN_HI_ODD, IDWT97_FILTER_HI_ODD, hiOdd);
            output[2 * i] = loEven[0] - ((hiEven[0] + hiEven[1] + 1) >> 1);
            output[2 * i + 1] = (9 * (loOdd[1] + loOdd[2]) - loOdd[0] - loOdd[3] + 8) >> 4;
            output[2 * i + 1] += (hiOdd[0] + hiOdd[4] - ((hiOdd[1] + hiOdd[3]) << 3) + 16) >> 5;
            output[2 * i + 1] += (23 * hiOdd[2] + 8) >> 4;
        }
        return true;
    }

    void IDWT(SubPicInfoMap& subPicInfo, SubBandMap& subBands, const SharedFrameBuffer& tmpFrameBuffer, const SharedFrameBuffer& rowBuffer, 
        std::vector<std::pair<uint32_t, uint32_t>> recSubPicSize, uint32_t bitDepth) {
        int32_t maxValue = (1 << bitDepth) - 1;
        for (auto color : COLORS) {
            auto info = subPicInfo[color];
            Pel* subBandLL = subBands[LL][color]->data();
            Pel* subBandLH = subBands[LH][color]->data();
            Pel* subBandHL = subBands[HL][color]->data();
            Pel* subBandHH = subBands[HH][color]->data();
            int horLen = static_cast<int>(info.w);
            int verLen = static_cast<int>(info.h);
            int subBandHorLen = horLen >> 1;
            int subBandVerLen = verLen >> 1;
            Pel* tmpL = tmpFrameBuffer->data();
            Pel* tmpH = tmpL + subBandHorLen;
            int strideSubBand = subBandHorLen;
            int strideTmp = horLen;
            for (int w = 0; w < subBandHorLen; ++w) {
                IDWT53(subBandLL, subBandLH, tmpL, verLen, strideSubBand, strideTmp);
                IDWT53(subBandHL, subBandHH, tmpH, verLen, strideSubBand, strideTmp);
                tmpL++;
                tmpH++;
                subBandLL++;
                subBandLH++;
                subBandHL++;
                subBandHH++;
            }

            tmpL = tmpFrameBuffer->data();
            tmpH = tmpL + subBandHorLen;
            auto rowBufferPtr = rowBuffer->data();
            auto recPicPtr = reinterpret_cast<PelStorage*>(info.picHeaderPtr) + info.y * info.strideW + info.x;
            for (int h = 0; h < recSubPicSize[color].second; ++h) {
#if DWT_97
                if (color == Y) {
                    IntDeconvDWT97(tmpL, tmpH, subBandHorLen, rowBufferPtr);
                } else 
#endif
                {
                    IDWT53(tmpL, tmpH, rowBufferPtr, horLen);
                }
                for (int w = 0; w < recSubPicSize[color].first; ++w) {
                    Pel recPixel = (rowBufferPtr[w] + (1 << (DWT_SHIFT - 1))) >> DWT_SHIFT;
                    Clip(recPixel, recPicPtr[w], 0, maxValue);
                }
                recPicPtr += info.strideW;
                tmpL += info.w;
                tmpH += info.w;
            }
        }
    }
}
