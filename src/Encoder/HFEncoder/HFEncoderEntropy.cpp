#include <cstring>
#include "HFEncoderEntropy.h"
#include "BasicTypes.h"
#include "Utils.h"

namespace ProVivid {

    static inline int BlockMaxAbsCoef(int* residual)
    {
        int max = 0;
        for (int i = 0; i < 16; i++)
        {
            if (abs(residual[i]) > max)
                max = abs(residual[i]);
        }
        return max;
    }

    static inline int IsAllZeroarray(int* residual, int length)
    {
        for (int i = 0; i < length; i++) {
            if (residual[i] != 0)
                return 0;
        }
        return 1;
    }

    const int UpbitsTable[5] = {4, 2, 1, 3, 5 };

    static inline int Log2Res(int value)
    {
        unsigned long ix, exp;
        float fx;
        fx = (float)value;
        ix = *(unsigned long*)&fx;
        exp = (ix >> 23) & 0xFF;
        return exp - 127;
    }

    inline int VLCTable0(int value)
    {
        if (abs(value) < 3)
            return UpbitsTable[value + 2];
        else
            return Log2Res(abs(value) - 2) + 6 + Log2Res(abs(value) - 2) + 1;
    }

    static inline int VLCTable1(int value)
    {
        if (abs(value) < 2)
            return abs(value) + 2;
        else if (abs(value) < 6)
            return abs(value) + 1;
        else
            return Log2Res(abs(value) - 4) + 5 + Log2Res(abs(value) - 4) + 1;
    }

    static inline int VLCTable2(int value)
    {
        if (value == 0)
            return  2;
        else if (abs(value) < 5)
            return 4;
        if (abs(value) < 7)
            return 5;
        else
            return Log2Res(abs(value) - 5) + 3 + Log2Res(abs(value) - 5) + 1;
    }

    static inline int VLCTable3(int value)
    {
        if (value == 0)
            return  2;
        else if (abs(value) < 32)
            return 3 + Log2Res(abs(value)) + 1;
        else
            return 2* Log2Res(abs(value));
    }

    inline void HFEncoderEntropy::WriteVLCTable0(int value)
    {
        int absValue = abs(value);
        if (absValue < 3){
            int prefix0Count = absValue * 2 + (value >> 15);
            for (int i = 0; i < prefix0Count; i++)
                write_u_v(1, 0, bitstream_);
            write_u_v(1, 1, bitstream_);
        }
        else{
            int prefix0Count = Log2Res(abs(value) - 2) + 5;
            for (int i = 0; i < prefix0Count; i++)
                write_u_v(1, 0, bitstream_);
            write_u_v(1, 1, bitstream_);

            int suffix = abs(value) - 2 - (1 << (prefix0Count - 5));
            for (int i = 0; i < prefix0Count - 5; ++i)
                write_u_v(1, suffix >> (prefix0Count - 6 - i) & 1, bitstream_);

            if (value > 0)
                write_u_v(1, 0, bitstream_);
            else
                write_u_v(1, 1, bitstream_);
        }
    }

    inline void HFEncoderEntropy::WriteVLCTable1(int value)
    {
        int absValue = abs(value);
        if (absValue == 0)
            write_u_v(2, 0, bitstream_);
        else
        {
            int prefix1Count = 0;
            switch (absValue)
            {
            case 1:
                write_u_v(2, 1, bitstream_);
                break;
            case 2:
                write_u_v(2, 2, bitstream_);
                break;
            case 3:
                write_u_v(3, 6, bitstream_);
                break;
            case 4:
                write_u_v(4, 14, bitstream_);
                break;
            case 5:
                write_u_v(5, 30, bitstream_);
                break;
            default:
            {
                prefix1Count = Log2Res(absValue - 4) + 4;
                for (int i = 0; i < prefix1Count; i++)
                    write_u_v(1, 1, bitstream_);
                write_u_v(1, 0, bitstream_);

                int suffix = absValue - 4 - (1 << (prefix1Count - 4));
                write_u_v(prefix1Count - 4, suffix, bitstream_);
            }
            }
            if (value > 0)
                write_u_v(1, 0, bitstream_);
            else
                write_u_v(1, 1, bitstream_);
        }
    }

    inline void HFEncoderEntropy::WriteVLCTable2(int value)
    {
        int absValue = abs(value);
        if (absValue == 0)
            write_u_v(2, 0, bitstream_);
        else
        {
            int prefix1Count = 0;
            switch (absValue)
            {
            case 1:
                write_u_v(3, 2, bitstream_);
                break;
            case 2:
                write_u_v(3, 3, bitstream_);
                break;
            case 3:
                write_u_v(3, 4, bitstream_);
                break;
            case 4:
                write_u_v(3, 5, bitstream_);
                break;
            case 5:
                write_u_v(4, 12, bitstream_);
                break;
            case 6:
                write_u_v(4, 13, bitstream_);
                break;
            default:
            {
                prefix1Count = Log2Res(absValue - 5) + 2;
                for (int i = 0; i < prefix1Count; i++)
                    write_u_v(1, 1, bitstream_);
                write_u_v(1, 0, bitstream_);

                int suffix = absValue - 5 - (1 << (prefix1Count - 2));
                write_u_v(prefix1Count - 2, suffix, bitstream_);
            }
            }
            if (value > 0)
                write_u_v(1, 0, bitstream_);
            else
                write_u_v(1, 1, bitstream_);
        }
    }

    inline void HFEncoderEntropy::WriteVLCTable3(int value)
    {
        int absValue = abs(value);
        if (absValue == 0)
            write_u_v(2, 0, bitstream_);
        else {
            unsigned int shiftBit = Log2Res(absValue);
            if (shiftBit < 5){
                write_u_v(3, shiftBit + 2, bitstream_);
                unsigned int remain_value = absValue - (1 << shiftBit);
                write_u_v(shiftBit, remain_value, bitstream_);
            }
            else {
                for (int i = 0; i < shiftBit - 2; i++)
                    write_u_v(1, 1, bitstream_);
                write_u_v(1, 0, bitstream_);

                unsigned int remainValue = absValue - (1 << shiftBit);
                write_u_v(shiftBit, remainValue, bitstream_);
            }
            if (value > 0)
                write_u_v(1, 0, bitstream_);
            else
                write_u_v(1, 1, bitstream_);
        }
    }

    void HFEncoderEntropy::HFEncPattern0001(int* resSubBlock)
    {
        if (resSubBlock[0] == 1 && resSubBlock[1] == 0 && resSubBlock[2] == 0 && resSubBlock[3] == 0)
            write_u_v(3, 0, bitstream_);
        else if (resSubBlock[0] == -1 && resSubBlock[1] == 0 && resSubBlock[2] == 0 && resSubBlock[3] == 0)
            write_u_v(3, 1, bitstream_);
        else if (resSubBlock[0] == 0 && resSubBlock[1] == 1 && resSubBlock[2] == 0 && resSubBlock[3] == 0)
            write_u_v(3, 2, bitstream_);
        else if (resSubBlock[0] == 0 && resSubBlock[1] == -1 && resSubBlock[2] == 0 && resSubBlock[3] == 0)
            write_u_v(3, 3, bitstream_);
        else if (resSubBlock[0] == 0 && resSubBlock[1] == 0 && resSubBlock[2] == 1 && resSubBlock[3] == 0)
            write_u_v(3, 4, bitstream_);
        else if (resSubBlock[0] == 0 && resSubBlock[1] == 0 && resSubBlock[2] == -1 && resSubBlock[3] == 0)
            write_u_v(3, 5, bitstream_);
        else if (resSubBlock[0] == 0 && resSubBlock[1] == 0 && resSubBlock[2] == 0 && resSubBlock[3] == 1)
            write_u_v(3, 6, bitstream_);
        else if (resSubBlock[0] == 0 && resSubBlock[1] == 0 && resSubBlock[2] == 0 && resSubBlock[3] == -1)
            write_u_v(3, 7, bitstream_);
    }

	HFEncoderEntropy::HFEncoderEntropy()
    {
	}

    HFEncoderEntropy::~HFEncoderEntropy()
	{
	}

    void HFEncoderEntropy::Set(Bitstream* bitstreamVlcHf, EncodingEnvironment* eeCabacHf, HighBandInfoContexts* highBandCtx)
    {
        bitstream_ = bitstreamVlcHf;
        eep_ = eeCabacHf;
        highBandCtx_ = highBandCtx;
    }

    void HFEncoderEntropy::HFEntropyFlag(BiContextTypePtr pBinCtx, int value)
    {
        biari_encode_symbol(eep_, value, pBinCtx);
    }

    void HFEncoderEntropy::HFEntropyCoeffBlock(int blockIdx, int* residual, EncDecBlockParams* blockParams)
    {
        int numCoeffInBlock = 16;
        int* coeff = residual + blockIdx * numCoeffInBlock;
        unsigned char subBlocksNotAllZero[4];

        for (int subBlokIndex = 0; subBlokIndex < 4; subBlokIndex++) {
            if (IsAllZeroarray(coeff + subBlokIndex * 4, 4))
                subBlocksNotAllZero[subBlokIndex] = 0;
            else
                subBlocksNotAllZero[subBlokIndex] = 1;
        }

        if (subBlocksNotAllZero[0] && subBlocksNotAllZero[1] && subBlocksNotAllZero[2] && subBlocksNotAllZero[3]) {  //1111subblocks
            HFEntropyFlag(&(highBandCtx_->zFlag[ZFLAG_ROW_MODEL_MODE_band0 + blockParams->bandIdx]), 1);
            int costVlc0 = 0;
            int costVlc1 = 0;
            int(*bitsVlc0)(int) = NULL;
            int(*bitsVlc1)(int) = NULL;
            void(HFEncoderEntropy::*writeVlc0)(int) = NULL;
            void(HFEncoderEntropy::*writeVlc1)(int) = NULL;

            if (blockParams->leftCoefMax[3 * blockParams->bandIdx + blockParams->colorComponent] <= 5) {
                bitsVlc0 = VLCTable0;
                bitsVlc1 = VLCTable1;
                writeVlc0 = &HFEncoderEntropy::WriteVLCTable0;
                writeVlc1 = &HFEncoderEntropy::WriteVLCTable1;
            }
            else if (blockParams->leftCoefMax[3 * blockParams->bandIdx + blockParams->colorComponent] <= 15) {
                bitsVlc0 = VLCTable1;
                bitsVlc1 = VLCTable2;
                writeVlc0 = &HFEncoderEntropy::WriteVLCTable1;
                writeVlc1 = &HFEncoderEntropy::WriteVLCTable2;
            }
            else {
                bitsVlc0 = VLCTable2;
                bitsVlc1 = VLCTable3;
                writeVlc0 = &HFEncoderEntropy::WriteVLCTable2;
                writeVlc1 = &HFEncoderEntropy::WriteVLCTable3;
            }

            for (int i = 0; i < numCoeffInBlock; i++) {
                costVlc0 += bitsVlc0(coeff[i]);
                costVlc1 += bitsVlc1(coeff[i]);
            }
            if (costVlc0 < costVlc1) {
                HFEntropyFlag(&(highBandCtx_->zFlag[TABLE_IDX_4x4_band0 + blockParams->bandIdx]), 0);
                for (int i = 0; i < numCoeffInBlock; i++)
                    (this->*writeVlc0)(coeff[i]);
            }
            else {
                HFEntropyFlag(&(highBandCtx_->zFlag[TABLE_IDX_4x4_band0 + blockParams->bandIdx]), 1);
                for (int i = 0; i < numCoeffInBlock; i++)
                    (this->*writeVlc1)(coeff[i]);
            }
        }
        else {
            HFEntropyFlag(&(highBandCtx_->zFlag[ZFLAG_ROW_MODEL_MODE_band0 + blockParams->bandIdx]), 0);
            for (int subBlokIndex = 0; subBlokIndex < 4; subBlokIndex++) {
                if (HF_NOHAD == blockParams->transformType)
                    HFEntropyFlag(&(highBandCtx_->zFlag[ZFLAG_ROW_MODEL_2x2_band0 + blockParams->bandIdx]), subBlocksNotAllZero[subBlokIndex]);
                else
                    HFEntropyFlag(&(highBandCtx_->zFlag[ZFLAG_ROW_MODEL_ACDC_1_band0_Y + 9 * subBlokIndex + blockParams->bandIdx * 3 + blockParams->colorComponent]), subBlocksNotAllZero[subBlokIndex]);
                
                if (subBlocksNotAllZero[subBlokIndex] == 1) {
                    int* resSubBlock = coeff + subBlokIndex * 4;
                    if (abs(resSubBlock[0]) + abs(resSubBlock[1]) + abs(resSubBlock[2]) + abs(resSubBlock[3]) == 1) { //0001 pattern
                        HFEntropyFlag(&(highBandCtx_->zFlag[PATTERN0001_MODEL_band0 + blockParams->bandIdx]), 1);
                        HFEncPattern0001(resSubBlock);
                    }
                    else {
                        HFEntropyFlag(&(highBandCtx_->zFlag[PATTERN0001_MODEL_band0 + blockParams->bandIdx]), 0);
                        int maxCoeffGt1 = 0;
                        for (int coefIdx = 0; coefIdx < 4; coefIdx++) {
                            if (abs(resSubBlock[coefIdx]) > 1)
                                maxCoeffGt1 = 1;
                        }
                        HFEntropyFlag(&(highBandCtx_->zFlag[MAX_COEF_GRT1_band0 + blockParams->bandIdx]), maxCoeffGt1);
                        if (maxCoeffGt1) {
                            for (int coefIdx = 0; coefIdx < 4; coefIdx++)
                                WriteVLCTable1(resSubBlock[coefIdx]);
                        }
                        else {
                            for (int coefIdx = 0; coefIdx < 4; coefIdx++) {
                                int absValue = abs(resSubBlock[coefIdx]);
                                write_u_v(1, absValue, bitstream_);
                                if(absValue)
                                    write_u_v(1, resSubBlock[coefIdx] < 0, bitstream_);
                            }
                        }
                    }
                }
            }
        }
    }

    void HFEncoderEntropy::HFEntropyCoeffGroupSet(int isLeftBoundaryMb, uint8_t bandIdx, uint8_t colorComponent, uint8_t transformType)
    {
        if (isLeftBoundaryMb)
            bgParams_.leftCoefMax[3 * (bandIdx - 1) + colorComponent] = {0};
        bgParams_.bandIdx = bandIdx - 1; //mapping
        bgParams_.colorComponent = colorComponent;
        if(colorComponent == Y)
            bgParams_.curBlockSize = 4;
        else
            bgParams_.curBlockSize = 2;
        bgParams_.transformType = transformType;
    }

    void HFEncoderEntropy::HFEntropyCoeffGroup(std::vector<int32_t>& residual, EncDecBlockGroupParams* bgParams, int hfTransformSkipEnable)
    {
        int32_t res[64] = {0};
        int blockPixels = 16;
        if (bgParams->transformType == HF_HAD) {
            for (int i = 0; i < bgParams->curBlockSize * blockPixels; i += blockPixels) {
                for (int j = 0; j < blockPixels; j++)
                    res[i + j] = residual[i + PermuteCoeffsTable[bgParams->bandIdx][j]];
            }
        }
        else
            memcpy(res, &residual[0], residual.size() * sizeof(int32_t));

        unsigned char blocksNotAllZero[4] = {0};
        for (int i = 0; i < bgParams->curBlockSize; i++)
            blocksNotAllZero[i] = !IsAllZeroarray(res + blockPixels * i, blockPixels);

        int startBlock = 0;
        int endBlock = startBlock + bgParams->curBlockSize;
        int groupNotAllZero = 0;
        for (int blockIdx = startBlock; blockIdx < endBlock; blockIdx++) {
            groupNotAllZero |= blocksNotAllZero[blockIdx];
        }
        HFEntropyFlag(&(highBandCtx_->zFlag[ZFLAG_BLOCK_MODEL_8x8nnz_band0_Y + bgParams->bandIdx * 3 + bgParams->colorComponent]), groupNotAllZero);
        
        if (groupNotAllZero) {
            int groupAllNotzero = 1;
            for (int blockIdx = startBlock; blockIdx < endBlock; blockIdx++) {
                groupAllNotzero &= blocksNotAllZero[blockIdx];
            }
            HFEntropyFlag(&(highBandCtx_->zFlag[ZFLAG_BLOCK_MODEL_8x8_allone_band0_Y + bgParams->bandIdx * 3 + bgParams->colorComponent]), groupAllNotzero);
            if (Y == bgParams->colorComponent && hfTransformSkipEnable)
                HFEntropyFlag(&(highBandCtx_->zFlag[ZFLAG_ROW_MODEL_TS_band0 + bgParams->bandIdx]), bgParams->transformType==HF_NOHAD);

            if (!groupAllNotzero) {
                int countZeroBlocks = 0;
                int countOneBlocks = 0;
                for (int blockIdx = startBlock; blockIdx < endBlock; blockIdx++) {
                    if(countZeroBlocks != endBlock - 1 && countOneBlocks != endBlock - 1)
                        HFEntropyFlag(&(highBandCtx_->zFlag[ZFLAG_MODEL_4x4_band0 + bgParams->bandIdx]), blocksNotAllZero[blockIdx]);
                    countZeroBlocks += (!blocksNotAllZero[blockIdx]);
                    countOneBlocks += blocksNotAllZero[blockIdx];
                }
            }
        }
        EncDecBlockParams blockParams;
        blockParams.bandIdx = bgParams->bandIdx;
        blockParams.colorComponent = bgParams->colorComponent;
        blockParams.leftCoefMax = bgParams->leftCoefMax;
        blockParams.transformType = bgParams->transformType;
        while (startBlock < endBlock) {
            if (blocksNotAllZero[startBlock] == 1) {
                HFEntropyCoeffBlock(startBlock, res, &blockParams);
                blockParams.leftCoefMax[3 * bgParams->bandIdx + bgParams->colorComponent] = BlockMaxAbsCoef(res + startBlock * 16); //block has 16 coeffs
            }
            else
                blockParams.leftCoefMax[3 * bgParams->bandIdx + bgParams->colorComponent] = 0;
            startBlock++;
        }
    }

    void HFEncoderEntropy::HFEntropyDeltaQp(int isLeftBoundaryMb, int subpicLumaQp, int& mbQp, int& qp_delta)
    {
        if (isLeftBoundaryMb)
            refQp_ = subpicLumaQp;
        Clip(mbQp - refQp_, qp_delta, -16, 15);
        write_se_v(qp_delta, bitstream_);
        mbQp = qp_delta + refQp_;
        refQp_ = mbQp;
    }
}