#ifndef LL_PU_CCLM_H
#define LL_PU_CCLM_H

#include "FrameBuffer.h"
#include "queue"
#include "Table.h"
#include "Const.h"
#include "Utils.h"
#include "algorithm"

namespace ProVivid {

    class LLPuCclm {
    public:
        LLPuCclm();
        ~LLPuCclm();
        void Set(uint32_t mbX, uint32_t mbY, uint8_t colorComponent);
        void PushNbr(SharedFrameBuffer llBandPicYRec, SharedFrameBuffer llBandPicURec, SharedFrameBuffer llBandPicVRec, uint32_t puYPixelIndex1, uint32_t puYPixelIndex2, bool avg, uint32_t puUVPixelIndex);
        void GetCclmScale(SharedFrameBuffer llBandPicYRec, SharedFrameBuffer llBandPicURec, SharedFrameBuffer llBandPicVRec, uint32_t puYPixelIndex, uint32_t lineYWidth, uint32_t puUVPixelIndex, uint32_t lineUVWidth);
        void ComLLCclmPred(SharedFrameBuffer llBandPicYRec, uint32_t puYPixelIndex, uint32_t lineYWidth, std::vector<int32_t>& intraPred);
    private:
        uint8_t colorComponent_;
        uint32_t shift1_;
        bool leftAvail_, upAvail_;
        int32_t diffY_,diffU_,diffV_;
        int32_t keyY_, keyUV_[2];
        int32_t slopeScale_[2];
        struct CclmNbr {
            Pel y, u, v;
            bool operator<(const CclmNbr &t) const {
                return (y < t.y);
            }
        };
        std::vector<CclmNbr> CclmNbr_;
    };
}

#endif // LL_PU_INTRA_H
