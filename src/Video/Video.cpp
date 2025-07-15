/* ====================================================================================================================

  The copyright in this software is being made available under the License included below.
  This software may be subject to other third party and contributor rights, including patent rights, and no such
  rights are granted under this license.

  Copyright (c) 2025, HUAWEI TECHNOLOGIES CO., LTD. All rights reserved.
  Copyright (c) 2025, PEKING UNIVERSITY. All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted only for
  the purpose of developing standards within Audio and Video Coding Standard Workgroup of China (AVS) and for testing and
  promoting such standards. The following conditions are required to be met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and
      the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and
      the following disclaimer in the documentation and/or other materials provided with the distribution.
    * The name of HUAWEI TECHNOLOGIES CO., LTD. may not be used to endorse or promote products derived from
      this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
  INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

* ====================================================================================================================
*/
#include <fstream>
#include "Const.h"
#include "Log.h"
#include "Utils.h"
#include "Video.h"

namespace HFM {
    Video::Video(PixelFormat pixelFormat, uint32_t frameCount, uint32_t width, uint32_t height, 
        const std::string& inputFilePath, AlphaInput alphaInput) : pixelFormat_(pixelFormat), frameCount_(frameCount) {
        inputAlphaFlag_ = alphaInput.inputAlphaFlag;
        inputAlpha16bitFlag_ = alphaInput.inputAlpha16bitFlag;
        inputAlphaFile_.open(alphaInput.inputAlphaFile, std::ifstream::binary);
        if (inputAlphaFile_.fail()) {
            LOGE("failed to open: %s\n", alphaInput.inputAlphaFile.c_str());
        } else {
            LOGI("open: %s\n", alphaInput.inputAlphaFile.c_str());
        }
        this->GetSize(width, height);
        InitFrameBuffer();
        inputVideo_.open(inputFilePath, std::ifstream::binary);
        if (inputVideo_.fail()) {
            LOGE("failed to open: %s\n", inputFilePath.c_str());
        } else {
            LOGI("open: %s\n", inputFilePath.c_str());
        }
    }

    Video::~Video() {
        if (inputVideo_.is_open()) {
            inputVideo_.close();
        }
    }

    SharedBufferStorage Video::GetCurrFrame() {
        return storageBuffer_;
    }

    SharedBufferStorage Video::MoveToNextFrame() {
        ReadOneFrame();
        return storageBuffer_;
    }

    void Video::GetSize(uint32_t width, uint32_t height) {
        uint32_t alignSize = MB_SIZE << 1;
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
                //formatShiftH_ = 1;
                break;
        }
        size_[CHROMA].w = size_[LUMA].w >> formatShiftW_;
        size_[CHROMA].h = size_[LUMA].h >> formatShiftH_;
        size_[CHROMA].strideW = size_[LUMA].strideW >> formatShiftW_;
        size_[CHROMA].strideH = size_[LUMA].strideH >> formatShiftH_;
        frameBufferPixels_ = size_[LUMA].strideW * size_[LUMA].strideH +
                             2 * (size_[CHROMA].strideW * size_[CHROMA].strideH);
        if (inputAlphaFlag_) {
            frameBufferPixels_ += size_[LUMA].strideW * size_[LUMA].strideH;
        }
    }

    void Video::InitFrameBuffer() {
        storageBuffer_ = std::make_unique<BufferStorage>(frameBufferPixels_, 0);
    }

    void Video::ReadOnePlane(PelStorage*& currPos, ImgBufSize size) {
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


    void Video::ReadOneAlphaPlane(PelStorage*& currPos, ImgBufSize size) {
        for (uint32_t h = 0; h < size.h; ++h) {
            if (inputAlpha16bitFlag_) {
                inputAlphaFile_.read((char *)currPos, size.w * sizeof(PelStorage));
                currPos += size.w;
            } else {
                for (uint32_t w = 0; w < size.w; ++w) {
                    inputAlphaFile_.read((char *)currPos, 1);
                    currPos++;
                }
            }
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

    void Video::ReadOneFrame() {
        auto currPos = storageBuffer_->data();
        ReadOnePlane(currPos, size_[LUMA]);
        ReadOnePlane(currPos, size_[CHROMA]);
        ReadOnePlane(currPos, size_[CHROMA]);
        if (inputAlphaFlag_) {
            ReadOneAlphaPlane(currPos, size_[LUMA]);
        }
        framePos_++;
    }

    uint32_t Video::FramePos() {
        return framePos_;
    }
}

