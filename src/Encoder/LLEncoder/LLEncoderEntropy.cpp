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
#include "LLEncoderEntropy.h"
#include "BasicTypes.h"
#include "Utils.h"
#include "Tool.h"

namespace ProVivid {
    LLEncoderEntropy::LLEncoderEntropy() {
    }

    LLEncoderEntropy::~LLEncoderEntropy() {
    }

    void LLEncoderEntropy::Set(Bitstream* bitstreamVlcLl, EncodingEnvironment* eeCabacLl, TextureInfoContexts* texCtx, MotionInfoContexts* motCtx) {
        bitstream_ = bitstreamVlcLl;
        eep_ = eeCabacLl;
        texCtx_ = texCtx;
        motCtx_ = motCtx;
    }

    void LLEncoderEntropy::GetCabcaState() {
        eeState_ = *eep_;
#if CABAC
        cabacLenState = *eep_->Ecodestrm_len;
#endif
        texCtxState_ = *texCtx_;
        motCtxState_ = *motCtx_;
        eep_->cabac_encoding = 0;
        bits_ = 0;
    }

    void LLEncoderEntropy::ResetCabcaState() {
        *eep_ = eeState_;
#if CABAC
        *eep_->Ecodestrm_len = cabacLenState;
#endif
        *texCtx_ = texCtxState_;
        *motCtx_ = motCtxState_;
        eep_->cabac_encoding = 1;
    }


    void LLEncoderEntropy::LLEntropyCoeff(uint8_t colorComponent, uint8_t puSize, uint8_t predMode, std::vector<int32_t>& residual) {
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

    void LLEncoderEntropy::LLEntropyPuSize(uint8_t isIntra,uint8_t colorComponent, uint8_t puSize) {
        if (isIntra && colorComponent == LUMA) {
#if CABAC
            bits_ += enc_writePuSize_CABAC(eep_, texCtx_, puSize);
#else
            if (eep_->cabac_encoding == 1) {
                write_u_v(1, puSize, bitstream_);
            } else {
                bits_ += 1;
            }
#endif
        }
    }

    void LLEncoderEntropy::LLEntropyIntraPredMode(uint8_t colorComponent, uint8_t predModeize) {
#if CABAC
        bits_ += enc_writePreMode_CABAC(eep_, texCtx_, predModeize, colorComponent);
#else
        if (eep_->cabac_encoding == 1) {
            write_u_v(2, predModeize, bitstream_);
        } else {
            bits_ += 2;
        }
#endif
    }

    void LLEncoderEntropy::LLEntropyMbMode(uint8_t isPframe, uint8_t mbMode) {
        if(isPframe != FRAME_I) {
#if CABAC
            bits_ += write_MB_mode_CABAC(eep_, motCtx_, mbMode);
#else
            if (eep_->cabac_encoding == 1) {
                write_u_v(1, mbMode, bitstream_);
            } else {
                bits_ += 1;
            }
#endif
        }
    }

    void LLEncoderEntropy::LLEntropyInterMode(uint8_t interNoResidualFlag) {
#if CABAC
        bits_ += write_inter_mode_CABAC(eep_, motCtx_, interNoResidualFlag);
#else
        if (eep_->cabac_encoding == 1) {
            write_u_v(1, interNoResidualFlag, bitstream_);
        } else {
            bits_ += 1;
        }
#endif
    }

    void LLEncoderEntropy::LLEntropyMvdMode(uint8_t mvdFlag) {
#if CABAC
        bits_ += write_inter_mvd_CABAC(eep_, motCtx_, mvdFlag);
#else
        if (eep_->cabac_encoding == 1) {
            write_u_v(1, mvdFlag, bitstream_);
        } else {
            bits_ += 1;
        }
#endif
    }

    void LLEncoderEntropy::LLEntropyMvd(int mvd, int mvp, int isMvdY, int cond) {
        bits_ += writeMVD_CABAC(bitstream_, eep_, motCtx_, mvd,  mvp, isMvdY, cond);
    }

    void LLEncoderEntropy::LLEntropyDeltaQp(int isLeftBoundaryMb, int subpicLumaQp, int& mbQp, int& qp_delta) {
        if (isLeftBoundaryMb) {
            refQp_ = subpicLumaQp;
        }
        Clip(mbQp - refQp_, qp_delta, -16, 15);
        write_se_v(qp_delta, bitstream_);
        mbQp = qp_delta + refQp_;
        refQp_ = mbQp;
    }
}
