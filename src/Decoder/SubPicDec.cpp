/* ====================================================================================================================

Copyright(c) 2025, HUAWEI TECHNOLOGIES CO., LTD.
Licensed under the Code Sharing Policy of the UHD World Association(the "Policy");
http://www.theuwa.com/UWA_Code_Sharing_Policy.pdf.
you may not use this file except in compliance with the Policy.
Unless agreed to in writing, software distributed under the Policy is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OF ANY KIND, either express or implied.
See the Policy for the specific language governing permissions and
limitations under the Policy.

* ====================================================================================================================
*/
#include "SubPicDec.h"

namespace ProVivid {
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
