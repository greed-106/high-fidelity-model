#include "EncoderQuant.h"

namespace ProVivid {
    EncoderQuant::EncoderQuant()
    {
    }

    EncoderQuant::~EncoderQuant()
    {
    }


    void EncoderQuant::Set(uint8_t puWidth, uint8_t  puHeight, uint8_t qp, uint32_t deadzone, uint32_t threshold1, uint32_t rdoqOffset, uint8_t quantRangeBit) {
        puWidth_ = puWidth;
        puHeight_ = puHeight;
        qp_ = qp;
        deadzone_ = deadzone;
        threshold1_ = threshold1;
        rdoqOffset_ = rdoqOffset;
        maxQuantVal_ = (1 << quantRangeBit) - 1;
        minQuantVal_ = -(1 << quantRangeBit);
    }


    void EncoderQuant::Quant(FrameBuffer& src, FrameBuffer& dst, uint8_t sizeIndex)
    {
        int32_t scale = QUANT_SCALE[sizeIndex][(qp_ + 12) % 8];
        int8_t qShift=QUANT_SHIFT+(qp_ + 12) / 8;
        int32_t deadzone = deadzone_ << (qShift - 9);
        int32_t threshold1 = threshold1_ << (qShift - 9);
        int32_t rdoqOffset = rdoqOffset_ << (qShift - 9);

        auto ptrSrc=&src[0];
        auto ptrDst = &dst[0];
        for (uint8_t i = 0; i < puHeight_; i++)
        {
            for (uint8_t j = 0; j < puWidth_; j++)
            {
                bool sign = ((*ptrSrc)<0);
                int32_t coeffTmp = *ptrSrc;
                Abs<int32_t,int32_t>(coeffTmp, coeffTmp);
                coeffTmp *=scale;
                if (coeffTmp < deadzone) {
                    coeffTmp = 0;
                } else if (coeffTmp < threshold1 + (1 << qShift)) {
                    coeffTmp = 1;
                }
                else {
                    coeffTmp = (coeffTmp + rdoqOffset) >> qShift;
                }
                (*ptrDst) = sign ? -coeffTmp : coeffTmp;
                Clip<int32_t, int32_t>((*ptrDst), (*ptrDst),minQuantVal_,maxQuantVal_);
                ptrSrc++;
                ptrDst++;
            }
        }
    }
}