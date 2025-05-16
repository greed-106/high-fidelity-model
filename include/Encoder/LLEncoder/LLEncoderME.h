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
