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
#include <cstring>
#include "HFDecoderEntropy.h"
#include "BasicTypes.h"
#include "Utils.h"
#include "Const.h"
#include "Tool.h"

namespace HFM {
    HFDecoderEntropy::HFDecoderEntropy() {
    }

    HFDecoderEntropy::~HFDecoderEntropy() {
    }

    static inline int BlockMaxAbsCoef(int* residual) {
        int max = 0;
        for (int i = 0; i < 16; i++) {
            if (abs(residual[i]) > max)
                max = abs(residual[i]);
        }
        return max;
    }

    void ReversePermuteBlockGroup(int blockIdx, int* residual, int bandIdx) {
        int* coeffRec = residual + blockIdx * 16;
        int tempCoeff[16];
        for (int i = 0; i < 16; i++) {
            tempCoeff[PermuteCoeffsTable[bandIdx][i]] = coeffRec[i];
        }
        for (int i = 0; i < 16; i++) {
            coeffRec[i] = tempCoeff[i];
        }
    }

    void HFDecoderEntropy::Set(Bitstream* bitstream, SubpicSyntaxInfo* subpicSyntaxInfo) {
        bitstreamCabacHf_ = {0};
#if CABAC_HF
        bitstreamCabacHf_.streamBuffer = bitstream->streamBuffer + (bitstream->frame_bitoffset >> 3) \
            + subpicSyntaxInfo->subpicLlCabacLength + subpicSyntaxInfo->subpicLlVlcLength;
        arideco_start_decoding(&de_, bitstreamCabacHf_.streamBuffer, 0, &bitstreamCabacHf_.read_len);
        InitContextsHf(0, &highBandCtx_);
#endif
        bitstreamVlcHf_ = {0};
        bitstreamVlcHf_.streamBuffer = bitstream->streamBuffer + (bitstream->frame_bitoffset >> 3) + subpicSyntaxInfo->subpicLlCabacLength \
            + subpicSyntaxInfo->subpicLlVlcLength + subpicSyntaxInfo->subpicHfCabacLength;
        bitstreamVlcHf_.bitstream_length = subpicSyntaxInfo->subpicLength - subpicSyntaxInfo->subpicLlCabacLength \
            - subpicSyntaxInfo->subpicLlVlcLength - subpicSyntaxInfo->subpicHfCabacLength;

        //residual_.resize(MB_SIZE*MB_SIZE);
        cbf_ = 0;
    }

    void HFDecoderEntropy::HFEntropyCoeffGroupSet(int isLeftBoundaryMb, uint8_t bandIdx, uint8_t colorComponent) {
        if (isLeftBoundaryMb)
            bgParams_.leftCoefMax[3 * (bandIdx - 1) + colorComponent] = 0;
        bgParams_.bandIdx = bandIdx - 1; //mapping
        bgParams_.colorComponent = colorComponent;
        if (colorComponent == Y)
            bgParams_.curBlockSize = 4;
        else
            bgParams_.curBlockSize = 2;
        bgParams_.transformType = HF_HAD;
        //residual_.assign(residual_.size(), 0);
    }

    int HFDecoderEntropy::HFEntropyFlag(BiContextTypePtr pBinCtx) {
#if CABAC_HF
        return biari_decode_symbol(&de_, pBinCtx);
#else
        return read_u_v(1, &bitstreamVlcHf_);
#endif
    }

    inline int HFDecoderEntropy::ReadVLCTable0() {
        int prefix = 0;
        while (read_u_v(1, &bitstreamVlcHf_) == 0) {
            prefix++;
        }

        if (prefix == 0)
            return 0;
        else if (prefix == 1)
            return -1;
        else if (prefix == 2)
            return 1;
        else if (prefix == 3)
            return -2;
        else if (prefix == 4)
            return 2;
        else {
            int value = prefix > 5 ? read_u_v(prefix - 5, &bitstreamVlcHf_) : 0;
            value += (1 << (prefix - 5)) + 2;
            if (read_u_v(1, &bitstreamVlcHf_))
                value = -value;
            return value;
        }
    }

    inline int HFDecoderEntropy::ReadVLCTable1() {
        int prefix = read_u_v(2, &bitstreamVlcHf_);
        if (prefix == 0)
            return 0;
        else if (prefix == 1) {
            if (read_u_v(1, &bitstreamVlcHf_))
                return -1;
            else
                return 1;
        } else if (prefix == 2) {
            if (read_u_v(1, &bitstreamVlcHf_))
                return -2;
            else
                return 2;
        } else {
            int countOne = 2;
            int value = 0;
            while (read_u_v(1, &bitstreamVlcHf_)) {
                countOne++;
            }
            if (countOne < 5) {
                value = countOne + 1;
                if (read_u_v(1, &bitstreamVlcHf_))
                    value = -value;
            } else {
                value = read_u_v(countOne - 4, &bitstreamVlcHf_) + (1 << (countOne - 4)) + 4;
                if (read_u_v(1, &bitstreamVlcHf_))
                    value = -value;
            }
            return value;
        }
    }

    inline int HFDecoderEntropy::ReadVLCTable2() {
        int prefix = read_u_v(2, &bitstreamVlcHf_);
        int value = 0;
        if (prefix == 0)
            return 0;
        else if (prefix == 1) {
            value = read_u_v(1, &bitstreamVlcHf_) + 1;
            if (read_u_v(1, &bitstreamVlcHf_))
                return -value;
            else
                return value;
        } else if (prefix == 2) {
            value = read_u_v(1, &bitstreamVlcHf_) + 3;
            if (read_u_v(1, &bitstreamVlcHf_))
                return -value;
            else
                return value;
        } else {
            int countOne = 0;
            while (read_u_v(1, &bitstreamVlcHf_)) {
                countOne++;
            }
            if (countOne == 0) {
                value = read_u_v(1, &bitstreamVlcHf_) + 5;
                if (read_u_v(1, &bitstreamVlcHf_))
                    value = -value;
            } else {
                value = read_u_v(countOne, &bitstreamVlcHf_) + (1 << countOne) + 5;
                if (read_u_v(1, &bitstreamVlcHf_))
                    value = -value;
            }
            return value;
        }
    }

    inline int HFDecoderEntropy::ReadVLCTable3() {
        int prefix = read_u_v(2, &bitstreamVlcHf_);
        if (prefix != 0) {
            int value = prefix * 2 + read_u_v(1, &bitstreamVlcHf_);
            if (value < 7) {
                int shiftBit = value - 2;
                int highBitSign = read_u_v(shiftBit + 1, &bitstreamVlcHf_);
                int vAbs = (highBitSign >> 1) + (1 << shiftBit);
                if (highBitSign & 1)
                    return -vAbs;
                else
                    return vAbs;
            } else {
                int countOne = 3;
                while (read_u_v(1, &bitstreamVlcHf_)) {
                    countOne++;
                }
                int shiftBit = countOne + 2;
                int highValue = read_u_v(shiftBit, &bitstreamVlcHf_);
                if (read_u_v(1, &bitstreamVlcHf_))
                    return -(highValue + (1 << shiftBit));
                else
                    return  highValue + (1 << shiftBit);
            }
        }
        else
            return 0;
    }

    inline void HFDecoderEntropy::HFDecPattern0001(int* resSubBlock, int patternIdx) {
        if (patternIdx == 0) {
            resSubBlock[0] = 1;
            resSubBlock[1] = 0;
            resSubBlock[2] = 0;
            resSubBlock[3] = 0;
        } else if (patternIdx == 1) {
            resSubBlock[0] = -1;
            resSubBlock[1] = 0;
            resSubBlock[2] = 0;
            resSubBlock[3] = 0;
        } else if (patternIdx == 2) {
            resSubBlock[0] = 0;
            resSubBlock[1] = 1;
            resSubBlock[2] = 0;
            resSubBlock[3] = 0;
        } else if (patternIdx == 3) {
            resSubBlock[0] = 0;
            resSubBlock[1] = -1;
            resSubBlock[2] = 0;
            resSubBlock[3] = 0;
        } else if (patternIdx == 4) {
            resSubBlock[0] = 0;
            resSubBlock[1] = 0;
            resSubBlock[2] = 1;
            resSubBlock[3] = 0;
        } else if (patternIdx == 5) {
            resSubBlock[0] = 0;
            resSubBlock[1] = 0;
            resSubBlock[2] = -1;
            resSubBlock[3] = 0;
        } else if (patternIdx == 6) {
            resSubBlock[0] = 0;
            resSubBlock[1] = 0;
            resSubBlock[2] = 0;
            resSubBlock[3] = 1;
        } else if (patternIdx == 7) {
            resSubBlock[0] = 0;
            resSubBlock[1] = 0;
            resSubBlock[2] = 0;
            resSubBlock[3] = -1;
        } else {
            printf("error patternIdx \n");
            exit(-1);
        }
    }

    void HFDecoderEntropy::HFEntropyCoeffBlock(int blockIdx, int* residual, EncDecBlockParams* blockParams) {
        int numCoeffInBlock = 16;
        int* coeff = residual + blockIdx * numCoeffInBlock;
        unsigned char subBlocksNotAllZero[4];

        bool isAllNonZeroSubblock = HFEntropyFlag(&(highBandCtx_.zFlag[ZFLAG_ROW_MODEL_MODE_band0 + blockParams->bandIdx]));

        if (isAllNonZeroSubblock) {  //1111subblocks
            bool modeIdx = HFEntropyFlag(&(highBandCtx_.zFlag[TABLE_IDX_4x4_band0 + blockParams->bandIdx]));

            int(HFDecoderEntropy::*readVlc0)() = NULL;
            int(HFDecoderEntropy::*readVlc1)() = NULL;
            if (blockParams->leftCoefMax[3 * blockParams->bandIdx + blockParams->colorComponent] <= 5) {
                readVlc0 = &HFDecoderEntropy::ReadVLCTable0;
                readVlc1 = &HFDecoderEntropy::ReadVLCTable1;
            } else if (blockParams->leftCoefMax[3 * blockParams->bandIdx + blockParams->colorComponent] <= 15) {
                readVlc0 = &HFDecoderEntropy::ReadVLCTable1;
                readVlc1 = &HFDecoderEntropy::ReadVLCTable2;
            } else {
                readVlc0 = &HFDecoderEntropy::ReadVLCTable2;
                readVlc1 = &HFDecoderEntropy::ReadVLCTable3;
            }

            if (modeIdx == 0) {
                for (int i = 0; i < numCoeffInBlock; i++) {
                    coeff[i] = (this->*readVlc0)();
                }
            } else {
                for (int i = 0; i < numCoeffInBlock; i++) {
                    coeff[i] = (this->*readVlc1)();
                }
            }
        } else {
            for (int subBlokIndex = 0; subBlokIndex < 4; subBlokIndex++) {
                if (HF_NOHAD == blockParams->transformType) {
                    subBlocksNotAllZero[subBlokIndex] = HFEntropyFlag(&(highBandCtx_.zFlag[ZFLAG_ROW_MODEL_2x2_band0 + blockParams->bandIdx]));
                } else {
                    subBlocksNotAllZero[subBlokIndex] = HFEntropyFlag(&(highBandCtx_.zFlag[ZFLAG_ROW_MODEL_ACDC_1_band0_Y + 9 * subBlokIndex + blockParams->bandIdx * 3 + blockParams->colorComponent]));
                }

                if (subBlocksNotAllZero[subBlokIndex]) {
                    bool is0001Pattern = HFEntropyFlag(&(highBandCtx_.zFlag[PATTERN0001_MODEL_band0 + blockParams->bandIdx]));
                    if (is0001Pattern) {
                        int patternIdx = read_u_v(3, &bitstreamVlcHf_);
                        HFDecPattern0001(coeff + 4 * subBlokIndex, patternIdx);
                    } else {
                        bool maxCoeffGt1 = HFEntropyFlag(&(highBandCtx_.zFlag[MAX_COEF_GRT1_band0 + blockParams->bandIdx]));
                        if (maxCoeffGt1) {
                            for (int coefIdx = 0; coefIdx < 4; coefIdx++) {
                                coeff[subBlokIndex * 4 + coefIdx] = ReadVLCTable1();
                            }
                        } else {
                            for (int coefIdx = 0; coefIdx < 4; coefIdx++) {
                                int absValue = read_u_v(1, &bitstreamVlcHf_);
                                if (absValue && read_u_v(1, &bitstreamVlcHf_)) {
                                    absValue = -absValue;
                                }
                                coeff[subBlokIndex * 4 + coefIdx] = absValue;
                            }
                        }
                    }

                }
            }
        }
    }


    void HFDecoderEntropy::HFEntropyCoeffGroup(uint32_t hfTransformSkipEnable, std::vector<int32_t>& residual) {
        int startBlock = 0;
        int endBlock = startBlock + bgParams_.curBlockSize;

        bool groupNotAllZero = HFEntropyFlag(&(highBandCtx_.zFlag[ZFLAG_BLOCK_MODEL_8x8nnz_band0_Y + bgParams_.bandIdx * 3 + bgParams_.colorComponent]));
        if (!groupNotAllZero) {
            for (int i = startBlock; i < bgParams_.curBlockSize; i++) {
                blocksNotAllZero[i] = 0;
                bgParams_.leftCoefMax[3 * bgParams_.bandIdx + bgParams_.colorComponent] = 0;
            }
        } else {
            bool groupAllNotzero = HFEntropyFlag(&(highBandCtx_.zFlag[ZFLAG_BLOCK_MODEL_8x8_allone_band0_Y + bgParams_.bandIdx * 3 + bgParams_.colorComponent]));
            if (bgParams_.colorComponent == Y && hfTransformSkipEnable) {
                bgParams_.transformType = HFEntropyFlag(&(highBandCtx_.zFlag[ZFLAG_ROW_MODEL_TS_band0 + bgParams_.bandIdx]));
            }

            if (groupAllNotzero) {
                for (int i = startBlock; i < bgParams_.curBlockSize; i++) {
                    blocksNotAllZero[i] = 1;
                }
            } else {
                int countZeroBlocks = 0;
                int countOneBlocks = 0;
                for (int blockIdx = startBlock; blockIdx < endBlock; blockIdx++) {
                    if (countZeroBlocks != endBlock - 1 && countOneBlocks != endBlock - 1) {
                        blocksNotAllZero[blockIdx] = HFEntropyFlag(&(highBandCtx_.zFlag[ZFLAG_MODEL_4x4_band0 + bgParams_.bandIdx]));
                    }
                    else if (countZeroBlocks == endBlock - 1) {
                        blocksNotAllZero[blockIdx] = 1;
                    } else {
                        blocksNotAllZero[blockIdx] = 0;
                    }
                    countZeroBlocks += (!blocksNotAllZero[blockIdx]);
                    countOneBlocks += blocksNotAllZero[blockIdx];
                }
            }

            EncDecBlockParams blockParams;
            blockParams.bandIdx = bgParams_.bandIdx;
            blockParams.colorComponent = bgParams_.colorComponent;
            blockParams.leftCoefMax = bgParams_.leftCoefMax;
            blockParams.transformType = bgParams_.transformType;
            while (startBlock < endBlock) {
                if (blocksNotAllZero[startBlock] == 1) {
                    HFEntropyCoeffBlock(startBlock, &residual[0], &blockParams);
                    blockParams.leftCoefMax[3 * bgParams_.bandIdx + bgParams_.colorComponent] = BlockMaxAbsCoef(&residual[0] + startBlock * 16); //block has 16 coeffs
                    if (bgParams_.transformType == HF_HAD || bgParams_.colorComponent != Y) {
                        ReversePermuteBlockGroup(startBlock, &residual[0], bgParams_.bandIdx);
                    }
                } else {
                    blockParams.leftCoefMax[3 * bgParams_.bandIdx + bgParams_.colorComponent] = 0;
                }
                startBlock++;
            }
        }
    }

    void HFDecoderEntropy::HFEntropyDecode(uint32_t qpDeltaEnable, uint32_t hfTransformSkipEnable, std::vector<int32_t>& residual) {
        if (qpDeltaEnable && bgParams_.colorComponent == Y && bgParams_.bandIdx == 0) {
            qpDelta = read_se_v(&bitstreamVlcHf_);
        } else {
            qpDelta = 0;
        }

        HFEntropyCoeffGroup(hfTransformSkipEnable, residual);

        for (int i = 0; i < bgParams_.curBlockSize; i++) {
            cbf_ |= blocksNotAllZero[i] << i;
        }
    }

}