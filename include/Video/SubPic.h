#ifndef SUB_PIC_H
#define SUB_PIC_H

#include <array>
#include <fstream>
#include <memory>
#include <unordered_map>
#include "BasicTypes.h"
#include "FrameBuffer.h"

namespace ProVivid {
    class SubPic {
    public:
        SubPic() = default;
        void Init(PixelFormat pixelFormat, uint32_t picWidth, uint32_t picHeight, uint32_t subPicWidth, uint32_t subPicHeight);
        ~SubPic() = default;
        ImgBufSize GetPicSizeRaw(ColorComp color);
        void GetFrame(SharedBufferStorage frameBuffer);
        void UpdateInfo(SharedBufferStorage storageBuffer);
        PixelFormat pixelFormat_{};
        uint32_t subPicCountHor_{};
        uint32_t subPicCountVer_{};
        std::vector<SubPicInfoMap> subPicInfo_;
        std::array<ImgBufSize, LUMA_CHROMA> picSize_{};
        std::array<ImgBufSize, LUMA_CHROMA> subPicSize_{};
        SharedBufferStorage storageBuffer_;
    private:
        static void GetAxis(uint32_t n, uint32_t fullLen, uint32_t uintLen, std::vector<uint32_t>& axis, std::vector<uint32_t>& length);
        void Divide();
        uint32_t formatShiftW_{};
        uint32_t formatShiftH_{};
        PelStorage* picHeaderPtr_[N_COLOR_COMP]{};
    };
}

#endif // SUB_PIC_H
