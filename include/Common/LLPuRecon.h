#ifndef LL_PU_RECON_H
#define LL_PU_RECON_H

#include "LLPuITrans.h"
#include "IQuant.h"
#include "FrameBuffer.h"
#include "Const.h"

namespace ProVivid {

    class LLPuRecon {
    public:
        LLPuRecon(std::shared_ptr<LLPuITrans> LLPUITrans, std::shared_ptr<IQuant> IQuant);
        ~LLPuRecon();
        void Set(uint8_t puWidth, uint8_t puHeight, uint32_t puPixelIndex, uint32_t lineWidth);
        void ComLLRecon(FrameBuffer& coeff, FrameBuffer& pred, SharedFrameBuffer llBandPicRec, uint8_t qp, uint8_t predMode);
        void ComLLRecon(FrameBuffer& pred, SharedFrameBuffer llBandPicRec);
    private:
        std::shared_ptr<LLPuITrans> LLPuITrans_;
        std::shared_ptr<IQuant> IQuant_;
        uint8_t puWidth_,puHeight_;
        uint32_t puPixelIndex_;
        uint32_t lineWidth_;
    };
}

#endif // LL_PU_RECON_H
