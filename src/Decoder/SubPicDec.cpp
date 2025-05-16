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
#include "SubPicDec.h"

namespace HFM {
    SubPicDec::SubPicDec(PixelFormat pixelFormat, uint32_t picWidth, uint32_t picHeight, uint32_t subPicWidth, uint32_t subPicHeight) {
        SubPic::Init(pixelFormat, picWidth, picHeight, subPicWidth, subPicHeight);
        subBandPixels_[Y] = (subPicSize_[LUMA].strideW * subPicSize_[LUMA].strideH) >> 2;
        subBandPixels_[U] = (subPicSize_[CHROMA].strideW * subPicSize_[CHROMA].strideH) >> 2;
        subBandPixels_[V] = subBandPixels_[U];
        for (const auto & band : SUB_BANDS) {
            for (const auto & subBandPixel : subBandPixels_) {
                subBands_[band].emplace_back(std::make_shared<FrameBuffer>(subBandPixel, 0));
                if (band == LL) {
                    subBandsRef_[band].emplace_back(std::make_shared<FrameBuffer>(subBandPixel, 0));
                }
            }
        }
        dwtRowBuffer_ = std::make_shared<FrameBuffer>(subPicSize_[LUMA].strideW, 0);
        dwtTransTmpBuffer_ = std::make_shared<FrameBuffer>(subPicSize_[LUMA].strideW * subPicSize_[LUMA].strideH, 0);
    }


    void SubPicDec::SetLLReference(SubPicInfoMap& subPicLLInfoRef) {
        for (const auto & color : COLORS) {
            auto info = subPicLLInfoRef[color];
            auto currPos = reinterpret_cast<PelStorage*>(info.picHeaderPtr) + info.y * info.strideW + info.x;
            Pel* subBandRefLL = subBandsRef_[LL][color]->data();
            for(int i = 0; i < info.h; i++) {
              for(int j = 0; j < info.w; j++) {
                subBandRefLL[j] = currPos[j];
              }
              subBandRefLL += info.w;
              currPos += info.strideW;
            }
        }
    }
}
