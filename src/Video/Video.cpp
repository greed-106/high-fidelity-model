#include <fstream>
#include "Const.h"
#include "Log.h"
#include "Utils.h"
#include "Video.h"

namespace ProVivid {
    Video::Video(PixelFormat pixelFormat, uint32_t frameCount, uint32_t width, uint32_t height, const std::string& inputFilePath) : pixelFormat_(pixelFormat), frameCount_(frameCount)
    {
        this->GetSize(width, height);
        InitFrameBuffer();
        inputVideo_.open(inputFilePath, std::ifstream::binary);
        if (inputVideo_.fail()) {
            LOGE("failed to open: %s\n", inputFilePath.c_str());
        } else {
            LOGI("open: %s\n", inputFilePath.c_str());
        }
    }

    Video::~Video()
    {
        if (inputVideo_.is_open()) {
            inputVideo_.close();
        }
    }

    SharedBufferStorage Video::GetCurrFrame()
    {
        return storageBuffer_;
    }

    SharedBufferStorage Video::MoveToNextFrame()
    {
        ReadOneFrame();
        return storageBuffer_;
    }

    void Video::GetSize(uint32_t width, uint32_t height)
    {
        uint32_t alignSize = MB_SIZE << (pixelFormat_ == PixelFormat::YUV444P10LE ? 0 : 1);
        size_[LUMA].w = width;
        size_[LUMA].h = height;
        size_[LUMA].strideW = Align(width, alignSize);
        size_[LUMA].strideH = Align(height, alignSize);
        switch (pixelFormat_) {
            case PixelFormat::YUV420P10LE:
                formatShiftW_ = 1;
                formatShiftH_ = 1;
                break;
            case PixelFormat::YUV422P10LE:
                formatShiftW_ = 1;
                break;
            case PixelFormat::YUV444P10LE:
                formatShiftH_ = 1;
                break;
        }
        size_[CHROMA].w = size_[LUMA].w >> formatShiftW_;
        size_[CHROMA].h = size_[LUMA].h >> formatShiftH_;
        size_[CHROMA].strideW = size_[LUMA].strideW >> formatShiftW_;
        size_[CHROMA].strideH = size_[LUMA].strideH >> formatShiftH_;
        frameBufferPixels_ = size_[LUMA].strideW * size_[LUMA].strideH +
                             2 * (size_[CHROMA].strideW * size_[CHROMA].strideH);
    }

    void Video::InitFrameBuffer()
    {
        storageBuffer_ = std::make_unique<BufferStorage>(frameBufferPixels_, 0);
    }

    void Video::ReadOnePlane(PelStorage*& currPos, ImgBufSize size)
    {
        for (uint32_t h = 0; h < size.h ; ++h) {
            inputVideo_.read((char *)currPos, size.w * sizeof(PelStorage));
            currPos += size.w;
            // padding right
            for (uint32_t w = 0; w < size.strideW - size.w; ++w) {
                *currPos = *(currPos - 1);
                currPos++;
            }
        }
        // padding bottom
        currPos -= size.strideW;
        for (uint32_t h = 0; h < size.strideH - size.h; ++h) {
            for (uint32_t w = 0; w < size.strideW; ++w) {
                currPos[w + size.strideW] = currPos[w];
            }
            currPos += size.strideW;
        }
        currPos += size.strideW;
    }

    void Video::ReadOneFrame()
    {
        auto currPos = storageBuffer_->data();
        ReadOnePlane(currPos, size_[LUMA]);
        ReadOnePlane(currPos, size_[CHROMA]);
        ReadOnePlane(currPos, size_[CHROMA]);
        framePos_++;
    }

    uint32_t Video::FramePos()
    {
        return framePos_;
    }
}

