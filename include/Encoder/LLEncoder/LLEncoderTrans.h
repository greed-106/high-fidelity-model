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
#ifndef LL_ENCODER_TRANS_H
#define LL_ENCODER_TRANS_H

#include <vector>
#include "FrameBuffer.h"
#include "Const.h"
#include "BasicTypes.h"
#include "Table.h"
#include "Utils.h"
#include "Tool.h"

namespace ProVivid {
    class LLEncoderTrans {
    public:
        LLEncoderTrans();
        ~LLEncoderTrans();
        void Set(uint8_t puWidth, uint8_t puHeight, uint8_t predMode);
        void LLTrans(FrameBuffer& predResidual, FrameBuffer& transCoeff);
        void LLEncTrans(FrameBuffer& src, FrameBuffer& dst, uint8_t is8, TRANS_TYPE transMode, uint8_t length, uint8_t shift);    
        void DST7W4(FrameBuffer& src, FrameBuffer& dst, uint8_t length, uint8_t shift);
        void DCT2W4(FrameBuffer& src, FrameBuffer& dst, uint8_t length, uint8_t shift);
        void DCT2W8(FrameBuffer& src, FrameBuffer& dst, uint8_t length, uint8_t shift);
    private:
        uint8_t puWidth_, puHeight_;
        FrameBuffer coeffTransHor_;
        //FrameBuffer coeffTransVer;
        TRANS_TYPE horTransMode_, verTransMode_;
    };
}

#endif // LL_ENCODER_TRANS_H
