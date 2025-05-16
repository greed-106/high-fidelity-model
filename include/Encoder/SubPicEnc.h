#ifndef SUB_PIC_ENC_H
#define SUB_PIC_ENC_H

#include <array>
#include <fstream>
#include <memory>
#include <unordered_map>
#include "SubPic.h"

namespace ProVivid {
    class SubPicEnc : public SubPic{
    public:
        SubPicEnc() = delete;
        SubPicEnc(PixelFormat pixelFormat, uint32_t picWidth, uint32_t picHeight, uint32_t subPicWidth, uint32_t subPicHeight);
        ~SubPicEnc() = default;
        void DWT(SubPicInfoMap& subPicInfo);
        void SetDWTSubPicPath(std::string dwtSubPicPath);
        void SetLLReference(SubPicInfoMap& subPicLLInfoRef);
        void WriteDWTSubPic(SubPicInfoMap& subPicInfo, const std::string& path);
        SubBandMap subBands_;
        SubBandMap subBandsRec_;
        SubBandMap subBandsRef_;
        SharedFrameBuffer dwtRowBuffer_;
        SharedFrameBuffer dwtTransTmpBuffer_;
    private:
        std::string dwtSubPicPath_;
        uint32_t subBandPixels_[N_COLOR_COMP]{};
    };
}

#endif // SUB_PIC_ENC_H
