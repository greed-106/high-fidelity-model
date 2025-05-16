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

