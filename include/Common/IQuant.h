#ifndef I_QUANT_H
#define I_QUANT_H

#include <vector>
#include "FrameBuffer.h"
#include "Table.h"
#include "Const.h"
#include "Utils.h"

namespace ProVivid {

    class IQuant {
    public:
        IQuant();
        ~IQuant();
        void Set(uint8_t puWidth, uint8_t  puHeight, uint8_t qp, uint32_t iQuantRangeBit);
        void ComIQuant(FrameBuffer& src, FrameBuffer& dst, uint8_t sizeIndex);
    private:
        uint8_t puWidth_,puHeight_;
        uint8_t qp_;
        int32_t maxIQuantVal_, minIQuantVal_;
    };
}

#endif // LL_PU_RECON_H
