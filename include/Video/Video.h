#ifndef VIDEO_H
#define VIDEO_H

#include <array>
#include <fstream>
#include <memory>
#include "BasicTypes.h"
#include "FrameBuffer.h"

namespace ProVivid {
    class Video {
    public:
        Video() = delete;
        Video(PixelFormat pixelFormat, uint32_t frameCount, uint32_t width, uint32_t height, const std::string& inputFilePath);
        ~Video();
        SharedBufferStorage GetCurrFrame();
        SharedBufferStorage MoveToNextFrame();
        uint32_t FramePos();
    private:
        void GetSize(uint32_t width, uint32_t height);
        void InitFrameBuffer();
        void ReadOneFrame();
        void ReadOnePlane(PelStorage*& currPos, ImgBufSize size);
        std::ifstream inputVideo_;
        PixelFormat pixelFormat_;
        uint32_t frameCount_{0};
        uint32_t formatShiftW_{0};
        uint32_t formatShiftH_{0};
        std::array<ImgBufSize, LUMA_CHROMA> size_{};
        uint32_t frameBufferPixels_{0};
        uint32_t framePos_{0};
        // W x H pixels in strideW x strideH buffer for encoder
        SharedBufferStorage storageBuffer_;
    };
}

#endif // VIDEO_H

