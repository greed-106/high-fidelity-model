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
#include "LLEncoderME.h"
#include "Wavelet.h"

namespace ProVivid {
    LLEncoderME::LLEncoderME() {
    }

    LLEncoderME::~LLEncoderME() {
    }

    MotionVector LLEncoderME::HalfPxlME(MotionVector intMv, float& minRDCost) {
        MotionVector bestMv = intMv;

        for (int32_t dy = -4; dy <= 3; dy++) {
            for (int32_t dx = -4; dx <= 3; dx++) {
                MotionVector mv(intMv.mvX + dx, intMv.mvY + dy);
                if (mv.mvX < -(MV_HOR_MAX << 1) || mv.mvX > (MV_HOR_MAX << 1) || mv.mvY < -(MV_VER_MAX << 1) || mv.mvY > (MV_VER_MAX << 1)) {
                    continue;
                }
                if (mv.mvX % 2 == 0 && mv.mvY % 2 == 0) {
                    continue;
                }
                uint32_t dist = 0;
                uint32_t rate = 0;
                float RDCost = 0;
                for (uint32_t compId = Y; compId < N_COLOR_COMP; compId++) {
                    uint32_t puWidth, puHeight;
                    if (compId == Y) {
                        puWidth = MB_SIZE;
                        puHeight = MB_SIZE;
                    } else {
                        puWidth = (MB_SIZE >> 1);
                        puHeight = MB_SIZE;
                    }
                    LLPuInter::Set(compId, puWidth, puHeight);
                    LLPuInter::InterPred(mv, *pred_, compId);
                    dist += Distortion(compId);
                }
                rate = Rate(mv);
                RDCost = RdCostCal(rate, dist, qp_[U]);
                if (RDCost < minRDCost) {
                    minRDCost = RDCost;
                    bestMv = mv;
                }
            }
        }
        return bestMv;
    }

    MotionVector LLEncoderME::IntPxlME(float& minRDCost) {
        MotionVector bestMv(0, 0);

        for (int32_t dy = -MV_VER_MAX; dy <= MV_VER_MAX; dy++) {
            for (int32_t dx = -MV_HOR_MAX; dx <= MV_HOR_MAX; dx++) {
                uint32_t dist = 0;
                uint32_t rate = 0;
                float RDCost = 0;
                MotionVector mv((dx << 1), (dy << 1));
                for (uint32_t compId = Y; compId < N_COLOR_COMP; compId++) {
                    uint32_t puWidth, puHeight;
                    if (compId == Y) {
                        puWidth = MB_SIZE;
                        puHeight = MB_SIZE;
                    } else {
                        puWidth = (MB_SIZE >> 1);
                        puHeight = MB_SIZE;
                    }
                    Set(compId, puWidth, puHeight);
                    InterPred(mv, *pred_, compId);
                    dist += Distortion(compId);
                }
                rate = Rate(mv);
                RDCost = RdCostCal(rate, dist, qp_[U]);
                if (RDCost < minRDCost) {
                    minRDCost = RDCost;
                    bestMv = mv;
                }
            }
        }
        return bestMv;
    }

    void LLEncoderME::SetOrg(uint32_t compId, SharedFrameBuffer llBandPic, uint32_t puPos, uint32_t orgStride) {
        org_[compId] = &(llBandPic->at(puPos));
        orgStride_[compId] = orgStride;
    }

    void LLEncoderME::SetMVP(MotionVector mv) {
        mvp_.mvX = mv.mvX;
        mvp_.mvY = mv.mvY;
    }

    void LLEncoderME::SetQP(uint8_t qp, int32_t cbQpOffset, int32_t crQpOffset) {
        qp_[Y] = qp;
        Clip<int8_t, uint8_t>(qp_[Y] + cbQpOffset, qp_[U], 0, 39);
        Clip<int8_t, uint8_t>(qp_[Y] + crQpOffset, qp_[V], 0, 39);
    }

    uint32_t LLEncoderME::EncodeUeLen(uint32_t val) {
        uint32_t len_i, len_c, nn;
        nn = ((val + 1) >> 1);
        for (len_i = 0; len_i < 16 && nn != 0; len_i++) {
            nn >>= 1;
        }
        len_c = (len_i << 1) + 1;
        return len_c;
    }

    uint32_t LLEncoderME::EncodeSeLen(int32_t val) {
        return EncodeUeLen(val <= 0 ? (-val * 2) : (val * 2 - 1));
    }

    uint32_t LLEncoderME::Rate(MotionVector mv) {
        MotionVector mvd;
        mvd.mvX = mv.mvX - mvp_.mvX;
        mvd.mvY = mv.mvY - mvp_.mvY;
        uint32_t len = EncodeSeLen(mvd.mvX) + EncodeSeLen(mvd.mvY);
        return len;
    }

    uint32_t LLEncoderME::Distortion(uint32_t compId) {
        uint32_t sse = 0;
        uint32_t tmp = 0;
        uint32_t orgIndex = 0;
        uint32_t predIndex = 0;
        for (uint8_t yPos = 0; yPos < puHeight_; ++yPos) {
            for (uint8_t xPos = 0; xPos < puWidth_; ++xPos) {
                tmp = (*pred_)[predIndex + xPos] - org_[compId][orgIndex + xPos];
                sse += tmp * tmp;
            }
            predIndex += puWidth_;
            orgIndex += orgStride_[compId];
        }
        return sse;
    }

}