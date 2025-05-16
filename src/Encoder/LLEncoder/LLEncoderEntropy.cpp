#include "LLEncoderEntropy.h"
#include "BasicTypes.h"
#include "Utils.h"

namespace ProVivid {
	LLEncoderEntropy::LLEncoderEntropy() {}

    LLEncoderEntropy::~LLEncoderEntropy() {}

    void LLEncoderEntropy::Set(Bitstream* bitstreamVlcLl, EncodingEnvironment* eeCabacLl, TextureInfoContexts* texCtx, MotionInfoContexts* motCtx)
    {
        bitstream_ = bitstreamVlcLl;
        eep_ = eeCabacLl;
        texCtx_ = texCtx;
        motCtx_ = motCtx;
    }

    void LLEncoderEntropy::GetCabcaState()
    {
        eeState_ = *eep_;
        cabacLenState = *eep_->Ecodestrm_len;
        texCtxState_ = *texCtx_;
        motCtxState_ = *motCtx_;
        eep_->cabac_encoding = 0;
        bits_ = 0;
    }

    void LLEncoderEntropy::ResetCabcaState()
    {
        *eep_ = eeState_;
        *eep_->Ecodestrm_len = cabacLenState;
        *texCtx_ = texCtxState_;
        *motCtx_ = motCtxState_;
        eep_->cabac_encoding = 1;
    }


    void LLEncoderEntropy::LLEntropyCoeff(uint8_t colorComponent, uint8_t puSize, uint8_t predMode, std::vector<int32_t>& residual)
    {
        int32_t* res = &residual[0];

        if (colorComponent != LUMA) {
            bits_ += enc_writeCoeff4x8_CABAC(bitstream_, eep_, texCtx_, res, colorComponent, predMode);
        } else {
            if (puSize == LUMA_PU_4x4) {
                bits_ += enc_writeCoeff4x4_CABAC(bitstream_, eep_, texCtx_, res, colorComponent, predMode);
            } else if (puSize == LUMA_PU_8x8) {
                bits_ += enc_writeCoeff8x8_CABAC(bitstream_, eep_, texCtx_, res, colorComponent, predMode);
            } else {
                 printf("Luma TU size only supports 4x4 and 8x8 for yuv422\n");
                 //TODO: open exit
                 exit(-1);
            }
        }
    }

    void LLEncoderEntropy::LLEntropyPuSize(uint8_t isIntra,uint8_t colorComponent, uint8_t puSize)
    {
        if (isIntra && colorComponent == LUMA) {
            bits_ += enc_writePuSize_CABAC(eep_, texCtx_, puSize);
        }
    }

    void LLEncoderEntropy::LLEntropyIntraPredMode(uint8_t colorComponent, uint8_t predModeize)
    {
        bits_ += enc_writePreMode_CABAC(eep_, texCtx_, predModeize, colorComponent);
    }

    void LLEncoderEntropy::LLEntropyMbMode(uint8_t isPframe, uint8_t mbMode)
    {
        if(isPframe != FRAME_I) {
            bits_ += write_MB_mode_CABAC(eep_, motCtx_, mbMode);
        }
    }

    void LLEncoderEntropy::LLEntropyInterMode(uint8_t interNoResidualFlag)
    {
        bits_ += write_inter_mode_CABAC(eep_, motCtx_, interNoResidualFlag);
    }

    void LLEncoderEntropy::LLEntropyMvdMode(uint8_t mvdFlag)
    {
        bits_ += write_inter_mvd_CABAC(eep_, motCtx_, mvdFlag);
    }

    void LLEncoderEntropy::LLEntropyMvd(int mvd, int mvp, int isMvdY, int cond)
    {
        bits_ += writeMVD_CABAC(bitstream_, eep_, motCtx_, mvd,  mvp, isMvdY, cond);
    }

    void LLEncoderEntropy::LLEntropyDeltaQp(int isLeftBoundaryMb, int subpicLumaQp, int& mbQp, int& qp_delta)
    {
        if (isLeftBoundaryMb) {
            refQp_ = subpicLumaQp;
        }
        Clip(mbQp - refQp_, qp_delta, -16, 15);
        write_se_v(qp_delta, bitstream_);
        mbQp = qp_delta + refQp_;
        refQp_ = mbQp;
    }
}
