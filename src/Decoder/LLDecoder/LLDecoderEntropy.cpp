#include "LLDecoderEntropy.h"
#include "BasicTypes.h"
#include "Utils.h"
#include "Const.h"

namespace ProVivid {
    LLDecoderEntropy::LLDecoderEntropy(Bitstream* bitstream, SubpicSyntaxInfo* subpicSyntaxInfo)
    {
        bitstreamCabacLl_ = {0};
        bitstreamCabacLl_.streamBuffer = bitstream->streamBuffer + (bitstream->frame_bitoffset >> 3);
        arideco_start_decoding(&de_, bitstreamCabacLl_.streamBuffer, 0, &bitstreamCabacLl_.read_len);
        InitContextsLl(subpicSyntaxInfo->subpicLlQpIndex, &texCtx_, &motCtx_);

        bitstreamVlcLl_ = {0};
        bitstreamVlcLl_.streamBuffer = bitstream->streamBuffer + (bitstream->frame_bitoffset >> 3) + subpicSyntaxInfo->subpicLlCabacLength;
        bitstreamVlcLl_.bitstream_length = subpicSyntaxInfo->subpicLlVlcLength;

        mbEntropyInfo_ = {0};
    }

    LLDecoderEntropy::~LLDecoderEntropy()
    {

    }


    void LLDecoderEntropy::LLEntropyMbInfo(uint32_t frameType, uint32_t qpDeltaEnable)
    {
        if (qpDeltaEnable) {
            mbEntropyInfo_.qpDelta = read_se_v(&bitstreamVlcLl_); // 0 order EG
        } else
        {
            mbEntropyInfo_.qpDelta = 0;
        }

        if (frameType == FRAME_P) {
            mbEntropyInfo_.mbMode = dec_read_MB_Mode_CABAC(&de_, &motCtx_);
        }

        if (mbEntropyInfo_.mbMode == MB_P) {    //inter mode
            mbEntropyInfo_.interNoResidualFlag = dec_read_inter_Mode_CABAC(&de_, &motCtx_);
            mbEntropyInfo_.interMvdFlag = dec_read_inter_mvd_CABAC(&de_, &motCtx_);
            if (mbEntropyInfo_.interMvdFlag) {
                mbEntropyInfo_.puMvdX = read_MVD_CABAC(&bitstreamVlcLl_, &de_, &motCtx_, 0, 1);
                mbEntropyInfo_.puMvdY = read_MVD_CABAC(&bitstreamVlcLl_, &de_, &motCtx_, 1, mbEntropyInfo_.puMvdX);
            } else {
                mbEntropyInfo_.puMvdX = 0;
                mbEntropyInfo_.puMvdY = 0;
            }

            mbEntropyInfo_.leftPuMvdX += mbEntropyInfo_.puMvdX;
            mbEntropyInfo_.leftPuMvdY += mbEntropyInfo_.puMvdY;
            mbEntropyInfo_.tuSizeLuma = LUMA_PU_8x8;
            mbEntropyInfo_.PredmodeLuma = OTHER;
            mbEntropyInfo_.PredmodeChroma = OTHER;
        } else {    //intra mode
            mbEntropyInfo_.tuSizeLuma = dec_readTuSize_CABAC(&de_, &texCtx_);
            mbEntropyInfo_.PredmodeLuma = dec_readPreMode_CABAC(&de_, &texCtx_, LUMA);
            mbEntropyInfo_.PredmodeChroma = dec_readPreMode_CABAC(&de_, &texCtx_, CHROMA);
            mbEntropyInfo_.leftPuMvdX = 0;
            mbEntropyInfo_.leftPuMvdY = 0;
        }

    }

    void LLDecoderEntropy::LLEntropyCoeff(int component, std::vector<int32_t>& residual)
    {
        if (component == Y) {
            if (mbEntropyInfo_.tuSizeLuma == LUMA_PU_4x4) {
                dec_readCoeff4x4_CABAC(&bitstreamVlcLl_, &de_, &texCtx_, &residual[0], component, mbEntropyInfo_.PredmodeLuma);
            } else if (!(mbEntropyInfo_.mbMode == MB_P && mbEntropyInfo_.interNoResidualFlag)) {
                dec_readCoeff8x8_CABAC(&bitstreamVlcLl_, &de_, &texCtx_, &residual[0], mbEntropyInfo_.PredmodeLuma);
            }
        } else
        {
            if (component == U) {
                dec_readCoeff4x8_CABAC(&bitstreamVlcLl_, &de_, &texCtx_, &residual[0], component, mbEntropyInfo_.PredmodeChroma);
            } else {
                dec_readCoeff4x8_CABAC(&bitstreamVlcLl_, &de_, &texCtx_, &residual[0], component, mbEntropyInfo_.PredmodeChroma);
            }
        }
    }
}