#ifndef SUB_PIC_DEC_H
#define SUB_PIC_DEC_H

#include <array>
#include <fstream>
#include <memory>
#include <unordered_map>
#include "SubPic.h"

namespace ProVivid {
    class SubPicDec : public SubPic{
    public:
        SubPicDec() = delete;
        SubPicDec(PixelFormat pixelFormat, uint32_t picWidth, uint32_t picHeight, uint32_t subPicWidth, uint32_t subPicHeight);
        ~SubPicDec() = default;
        void SetLLReference(SubPicInfoMap& subPicLLInfoRef);
        SubBandMap subBands_;
        SubBandMap subBandsRef_;
        SharedFrameBuffer dwtRowBuffer_;
        SharedFrameBuffer dwtTransTmpBuffer_;
    private:
        uint32_t subBandPixels_[N_COLOR_COMP]{};
    };
}

#endif // SUB_PIC_DEC_H
