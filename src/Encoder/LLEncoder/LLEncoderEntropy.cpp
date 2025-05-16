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
#include "LLEncoderEntropy.h"
#include "BasicTypes.h"
#include "Utils.h"
#include "Tool.h"

namespace HFM {
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
