#include "HFTransITrans.h"

namespace ProVivid {
    HFTransITrans::HFTransITrans()
    {
    }
    HFTransITrans::~HFTransITrans()
    {
    }

    void HFTransITrans::Set(uint8_t colorComponent, uint8_t isITrans) {
        pixNum_ = (colorComponent == Y) ? (MB_SIZE*MB_SIZE) : (MB_SIZE*MB_SIZE >> 1);
        isITrans_ = isITrans;
    }

    void HFTransITrans::ComHFTransITrans(FrameBuffer& src, FrameBuffer& dst) {
        auto ptrSrc = &src[0];
        Pel tmp[MB_SIZE*MB_SIZE];
        for (uint32_t blockNum = 0; blockNum < (pixNum_ >> 2); blockNum++) {
            tmp[blockNum * 4 + 0] = ptrSrc[0] + ptrSrc[1] + ptrSrc[2] + ptrSrc[3];
            tmp[blockNum * 4 + 1] = ptrSrc[0] - ptrSrc[1] + ptrSrc[2] - ptrSrc[3];
            tmp[blockNum * 4 + 2] = ptrSrc[0] + ptrSrc[1] - ptrSrc[2] - ptrSrc[3];
            tmp[blockNum * 4 + 3] = ptrSrc[0] - ptrSrc[1] - ptrSrc[2] + ptrSrc[3];
            ptrSrc += 4;
        }
        auto ptrDst = &dst[0];
        for (uint32_t i = 0; i < pixNum_; i++) {
            if (tmp[i] < 0) {
                ptrDst[i] = (tmp[i] >> 1);
            } else {
                ptrDst[i] = ((tmp[i] + 1) >> 1);
            }
        }
        if (isITrans_) {
            int32_t maxHFITransVal = (1 << HF_ITRANS_DYNAMIC_BIT) - 1;
            int32_t minHFITransVal = -(1 << HF_ITRANS_DYNAMIC_BIT);
            for (uint32_t i = 0; i < pixNum_; i++) {
                Clip<Pel, Pel>(ptrDst[i], ptrDst[i], minHFITransVal, maxHFITransVal);
            }
        }
    }
}

