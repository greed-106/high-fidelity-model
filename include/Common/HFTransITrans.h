#ifndef HF_TRANS_I_TRANS_H
#define HF_TRANS_I_TRANS_H

#include <vector>
#include "FrameBuffer.h"
#include "Const.h"
#include "Utils.h"

namespace ProVivid {

    class HFTransITrans {
    public:
        HFTransITrans();
        ~HFTransITrans();
        void Set(uint8_t colorComponent, uint8_t isITrans);
        void ComHFTransITrans(FrameBuffer& src, FrameBuffer& dst);
    private:
        uint32_t pixNum_;
        uint8_t isITrans_;
    };
}

#endif // LL_PU_RECON_H
