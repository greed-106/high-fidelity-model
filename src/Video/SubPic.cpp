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
#include <string>
#include "Const.h"
#include "SubPic.h"
#include "Utils.h"

namespace ProVivid {
    void SubPic::Init(PixelFormat pixelFormat, uint32_t picWidth, uint32_t picHeight, uint32_t subPicWidth, uint32_t subPicHeight) {
        pixelFormat_ = pixelFormat;
        uint32_t alignSize = MB_SIZE << (pixelFormat_ == PixelFormat::YUV444P10LE ? 0 : 1);
        picSize_[LUMA].w = picWidth;
        picSize_[LUMA].h = picHeight;
        picSize_[LUMA].strideW = Align(picWidth, alignSize);
        picSize_[LUMA].strideH = Align(picHeight, alignSize);
        subPicSize_[LUMA].w = subPicWidth;
        subPicSize_[LUMA].h = subPicHeight;
        subPicSize_[LUMA].strideW = subPicWidth;
        subPicSize_[LUMA].strideH = subPicHeight;
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
        picSize_[CHROMA].w = picSize_[LUMA].w >> formatShiftW_;
        picSize_[CHROMA].h = picSize_[LUMA].h >> formatShiftH_;
        picSize_[CHROMA].strideW = picSize_[LUMA].strideW >> formatShiftW_;
        picSize_[CHROMA].strideH = picSize_[LUMA].strideH >> formatShiftH_;
        subPicSize_[CHROMA].w = subPicSize_[LUMA].w >> formatShiftW_;
        subPicSize_[CHROMA].h = subPicSize_[LUMA].h >> formatShiftH_;
        subPicSize_[CHROMA].strideW = subPicSize_[LUMA].strideW >> formatShiftW_;
        subPicSize_[CHROMA].strideH = subPicSize_[LUMA].strideH >> formatShiftH_;
        this->Divide();
    }

    void SubPic::GetAxis(uint32_t n, uint32_t fullLen, uint32_t uintLen, std::vector<uint32_t>& axis, std::vector<uint32_t>& length) {
        uint32_t currPos = 0;
        for (int i = 0; i < n; ++i) {
            axis.emplace_back(currPos);
            uint32_t remainingX = fullLen - currPos;
            uint32_t currLen = std::min(remainingX, uintLen);
            length.emplace_back(currLen);
            currPos += uintLen;
        }
    }

    void SubPic::GetFrame(SharedBufferStorage storageBuffer) {
        storageBuffer_ = std::move(storageBuffer);
    }

    void SubPic::UpdateInfo(SharedBufferStorage storageBuffer) {
        this->GetFrame(std::move(storageBuffer));
        if (storageBuffer_->empty()) {
            LOGE("storage buffer not initialized.");
        } else {
            picHeaderPtr_[Y] = storageBuffer_->data();
            picHeaderPtr_[U] = picHeaderPtr_[Y] + picSize_[LUMA].strideW * picSize_[LUMA].strideH;
            picHeaderPtr_[V] = picHeaderPtr_[U] + picSize_[CHROMA].strideW * picSize_[CHROMA].strideH;
        }
        uint32_t subPicIndex = 0;
        for (int j = 0; j < subPicCountVer_; ++j) {
            for (int i = 0; i < subPicCountHor_; ++i) {
                for (const auto & color : COLORS) {
                    subPicInfo_[subPicIndex][color].picHeaderPtr = reinterpret_cast<void*>(picHeaderPtr_[color]);
                }
                subPicIndex++;
            }
        }
    }

    void SubPic::Divide() {
        subPicCountHor_ = (picSize_[LUMA].strideW + subPicSize_[LUMA].w - 1) / subPicSize_[LUMA].w;
        subPicCountVer_ = (picSize_[LUMA].strideH + subPicSize_[LUMA].h - 1) / subPicSize_[LUMA].h;

        std::vector<uint32_t> axisXLuma;
        std::vector<uint32_t> subPicWidthLuma;
        GetAxis(subPicCountHor_, picSize_[LUMA].strideW, subPicSize_[LUMA].w, axisXLuma, subPicWidthLuma);
        std::vector<uint32_t> axisYLuma;
        std::vector<uint32_t> subPicHeightLuma;
        GetAxis(subPicCountVer_, picSize_[LUMA].strideH, subPicSize_[LUMA].h, axisYLuma, subPicHeightLuma);

        std::vector<uint32_t> axisXChroma;
        std::vector<uint32_t> subPicWidthChroma;
        GetAxis(subPicCountHor_, picSize_[CHROMA].strideW, subPicSize_[CHROMA].w, axisXChroma, subPicWidthChroma);
        std::vector<uint32_t> axisYChroma;
        std::vector<uint32_t> subPicHeightChroma;
        GetAxis(subPicCountVer_, picSize_[CHROMA].strideH, subPicSize_[CHROMA].h, axisYChroma, subPicHeightChroma);

        std::vector<uint32_t>* axisX[] = {&axisXLuma, &axisXChroma, &axisXChroma};
        std::vector<uint32_t>* axisY[] = {&axisYLuma, &axisYChroma, &axisYChroma};
        std::vector<uint32_t>* subPicWidth[] = {&subPicWidthLuma, &subPicWidthChroma, &subPicWidthChroma};
        std::vector<uint32_t>* subPicHeight[] = {&subPicHeightLuma, &subPicHeightChroma, &subPicHeightChroma};
        uint32_t picStrideW[] = {picSize_[LUMA].strideW, picSize_[CHROMA].strideW, picSize_[CHROMA].strideW};
        uint32_t picStrideH[] = {picSize_[LUMA].strideH, picSize_[CHROMA].strideH, picSize_[CHROMA].strideH};

        uint32_t subPicIndex = 0;
        for (int j = 0; j < subPicCountVer_; ++j) {
            for (int i = 0; i < subPicCountHor_; ++i) {
                SubPicBorder border;
                if (i == subPicCountHor_ - 1 && j == subPicCountVer_ - 1) {
                    border = SubPicBorder::CORNER;
                } else if (i == subPicCountHor_ - 1) {
                    border = SubPicBorder::RIGHT;
                } else if (j == subPicCountVer_ - 1) {
                    border = SubPicBorder::BOTTOM;
                } else {
                    border = SubPicBorder::NONE;
                }
                SubPicInfoMap infoMap;
                for (const auto & color : COLORS) {
                    SubPicInfo info = {
                        subPicIndex,
                        (*axisX[color])[i],
                        (*axisY[color])[j],
                        (*subPicWidth[color])[i],
                        (*subPicHeight[color])[j],
                        picStrideW[color],
                        picStrideH[color],
                        border,
                        nullptr
                    };
                    infoMap[color] = info;
                }
                subPicInfo_.emplace_back(infoMap);
                subPicIndex++;
            }
        }
    }

    ImgBufSize SubPic::GetPicSizeRaw(ColorComp color) {
        if (color == Y) {
            return picSize_[LUMA];
        } else {
            return picSize_[CHROMA];
        }
    }
}
