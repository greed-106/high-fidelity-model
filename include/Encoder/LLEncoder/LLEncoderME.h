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
#ifndef LL_ENCODER_ME_H
#define LL_ENCODER_ME_H

#include <vector>
#include <fstream>
#include "Const.h"
#include "BasicTypes.h"
#include "Table.h"
#include "LLPuInter.h"
#include "Utils.h"

namespace ProVivid {
    class LLEncoderME : public LLPuInter {
    public:
        LLEncoderME();
        ~LLEncoderME();
        void SetOrg(uint32_t compId, SharedFrameBuffer llBandPic, uint32_t puPos, uint32_t orgStride);
        void SetMVP(MotionVector MV);
        void SetQP(uint8_t qp, int32_t cbQpOffset, int32_t crQpOffset);
        MotionVector IntPxlME(float& minRDCost);
        MotionVector HalfPxlME(MotionVector intMv, float& minRDCost);
        uint32_t EncodeSeLen(int32_t val);
        uint32_t EncodeUeLen(uint32_t val);
        uint32_t Rate(MotionVector mv);
        uint32_t Distortion(uint32_t compId);
    private:
        uint8_t qp_[N_COLOR_COMP];
        MotionVector mvp_;
        Pel*  org_[N_COLOR_COMP];
        uint32_t orgStride_[N_COLOR_COMP];
    };
}

#endif // LL_ENCODER_ME_H
