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
#include "LLDecoderEntropy.h"
#include "BasicTypes.h"
#include "Utils.h"
#include "Const.h"
#include "Tool.h"

namespace HFM {
    LLDecoderEntropy::LLDecoderEntropy(Bitstream* bitstream, SubpicSyntaxInfo* subpicSyntaxInfo) {
        bitstreamCabacLl_ = {0};
#if CABAC
        bitstreamCabacLl_.streamBuffer = bitstream->streamBuffer + (bitstream->frame_bitoffset >> 3);
        arideco_start_decoding(&de_, bitstreamCabacLl_.streamBuffer, 0, &bitstreamCabacLl_.read_len);
        InitContextsLl(&texCtx_, &motCtx_);
#endif
        bitstreamVlcLl_ = {0};
        bitstreamVlcLl_.streamBuffer = bitstream->streamBuffer + (bitstream->frame_bitoffset >> 3) + subpicSyntaxInfo->subpicLlCabacLength;
        bitstreamVlcLl_.bitstream_length = subpicSyntaxInfo->subpicLlVlcLength;

        mbEntropyInfo_ = {0};
    }

    LLDecoderEntropy::~LLDecoderEntropy() {

    }


    void LLDecoderEntropy::LLEntropyMbInfo(uint32_t frameType, uint32_t qpDeltaEnable) {
        if (qpDeltaEnable) {
            mbEntropyInfo_.qpDelta = read_se_v(&bitstreamVlcLl_); // 0 order EG
        } else {
            mbEntropyInfo_.qpDelta = 0;
        }

        if (frameType == FRAME_P) {
#if CABAC
            mbEntropyInfo_.mbMode = dec_read_MB_Mode_CABAC(&de_, &motCtx_);
#else
            mbEntropyInfo_.mbMode = read_u_v(1, &bitstreamVlcLl_);
#endif
        }

        if (mbEntropyInfo_.mbMode == MB_P) {    //inter mode
#if CABAC
            mbEntropyInfo_.interNoResidualFlag = dec_read_inter_Mode_CABAC(&de_, &motCtx_);
            mbEntropyInfo_.interMvdFlag = dec_read_inter_mvd_CABAC(&de_, &motCtx_);
#else
            mbEntropyInfo_.interNoResidualFlag = read_u_v(1, &bitstreamVlcLl_);
            mbEntropyInfo_.interMvdFlag = read_u_v(1, &bitstreamVlcLl_);
#endif
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
#if CABAC
            mbEntropyInfo_.tuSizeLuma = dec_readTuSize_CABAC(&de_, &texCtx_);
            mbEntropyInfo_.PredmodeLuma = dec_readPreMode_CABAC(&de_, &texCtx_, LUMA);
            mbEntropyInfo_.PredmodeChroma = dec_readPreMode_CABAC(&de_, &texCtx_, CHROMA);
#else
            mbEntropyInfo_.tuSizeLuma = read_u_v(1, &bitstreamVlcLl_);
            mbEntropyInfo_.PredmodeLuma = read_u_v(2, &bitstreamVlcLl_);
            mbEntropyInfo_.PredmodeChroma = read_u_v(2, &bitstreamVlcLl_);
#endif
            mbEntropyInfo_.leftPuMvdX = 0;
            mbEntropyInfo_.leftPuMvdY = 0;
        }

    }

    void LLDecoderEntropy::LLEntropyCoeff(int component, std::vector<int32_t>& residual) {
        if (component == Y) {
            if (mbEntropyInfo_.tuSizeLuma == LUMA_PU_4x4) {
                dec_readCoeff4x4_CABAC(&bitstreamVlcLl_, &de_, &texCtx_, &residual[0], component, mbEntropyInfo_.PredmodeLuma);
            } else if (!(mbEntropyInfo_.mbMode == MB_P && mbEntropyInfo_.interNoResidualFlag)) {
                dec_readCoeff8x8_CABAC(&bitstreamVlcLl_, &de_, &texCtx_, &residual[0], mbEntropyInfo_.PredmodeLuma);
            }
        } else {
            if (component == U) {
                dec_readCoeff4x8_CABAC(&bitstreamVlcLl_, &de_, &texCtx_, &residual[0], component, mbEntropyInfo_.PredmodeChroma);
            } else {
                dec_readCoeff4x8_CABAC(&bitstreamVlcLl_, &de_, &texCtx_, &residual[0], component, mbEntropyInfo_.PredmodeChroma);
            }
        }
    }
}