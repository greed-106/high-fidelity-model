#ifndef LL_ENCODER_TRANS_H
#define LL_ENCODER_TRANS_H

#include <vector>
#include "FrameBuffer.h"
#include "Const.h"
#include "BasicTypes.h"
#include "Table.h"
#include "Utils.h"

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
