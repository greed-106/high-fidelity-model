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
#include <fstream>
#include "LLPuInter.h"
#include "Wavelet.h"

namespace ProVivid {
    LLPuInter::LLPuInter() {
        pred_ = std::make_shared<PixelBuffer>(MB_SIZE*MB_SIZE, 0);
        for (uint32_t i = 0; i < N_COLOR_COMP; i++) {
            refWinYUV_[i] = std::make_shared<PixelBuffer>(REF_HOR_SIZE*REF_VER_SIZE, 0);
        }
    };

    LLPuInter::~LLPuInter() {
    };

    void LLPuInter::Set(uint32_t compId, uint32_t puWidth, uint32_t puHeight) {
        this->puWidth_ = puWidth;
        this->puHeight_ = puHeight;
        this->refWin_ = refWinYUV_[compId];

        if (compId == Y) {
            this->refWinWidth_ = 2 * MV_HOR_MAX + puWidth;
            this->refWinHeight_ = 2 * MV_VER_MAX + puHeight;
            this->refPadWidth_ = MV_HOR_MAX;
            this->refPadHeight_ = MV_VER_MAX;
            this->refWinStride_ = this->refWinWidth_;
            this->refWinOrgIdx_ = MV_VER_MAX * this->refWinStride_ + MV_HOR_MAX;
        } else {
            this->refWinWidth_ = (MV_HOR_MAX + 1) + puWidth;
            this->refWinHeight_ = 2 * MV_VER_MAX + puHeight;
            this->refPadWidth_ = (MV_HOR_MAX + 1) >> 1;
            this->refPadHeight_ = MV_VER_MAX;
            this->refWinStride_ = this->refWinWidth_;
            this->refWinOrgIdx_ = MV_VER_MAX * this->refWinStride_ + ((MV_HOR_MAX + 1) >> 1);
        }
    };


    void LLPuInter::GeRefWindow(SharedFrameBuffer llBandPicRec_, uint32_t llBandWidth_, uint32_t compId, uint32_t mbY, uint32_t mbX, bool rowFirstFlag, bool rowLastFlag, bool colFirstFlag, bool colLastFlag) {
        uint32_t lineWidth;
        uint32_t componentShiftX = 0;
        if (compId != Y) {
            componentShiftX = 1;
        }
        lineWidth = llBandWidth_;
        uint32_t srcIdx = mbY * MB_SIZE * lineWidth + mbX * (MB_SIZE >> componentShiftX);
        uint32_t dstIdx = refWinOrgIdx_;
        int32_t srcStride = lineWidth;
        int32_t dstStride = refWinStride_;

        Pel *src, *dst;
        Pel *tmpSrc, *tmpDst;
        src = &(*llBandPicRec_)[srcIdx];

        dst = &(*refWin_)[dstIdx];
        // copy central
        GetRefPixel(src, dst, srcStride, dstStride, puHeight_, puWidth_);

        //copy right
        if (!colLastFlag) {
            tmpSrc = src + puWidth_;
            tmpDst = dst + puWidth_;
            GetRefPixel(tmpSrc, tmpDst, srcStride, dstStride, puHeight_, refPadWidth_);
        } else {
            // padding right
            for (int i = 0; i < puHeight_; i++) {
                for (int j = 0; j < refPadWidth_; j++) {
                    dst[puWidth_ + j + i * dstStride] = dst[i * dstStride + puWidth_ - 1];
                }
            }
        }

        //copy left
        if (!colFirstFlag) {
            tmpSrc = src - refPadWidth_;
            tmpDst = dst - refPadWidth_;
            GetRefPixel(tmpSrc, tmpDst, srcStride, dstStride, puHeight_, refPadWidth_);
        } else {
            // padding left
            for (int i = 0; i < puHeight_; i++) {
                for (int j = 0; j < refPadWidth_; j++) {
                    dst[-refPadWidth_ + j + i * dstStride] = dst[i * dstStride];
                }
            }
        }


        //copy top
        if (!rowFirstFlag) {
            tmpSrc = src - refPadHeight_ * srcStride;
            tmpDst = dst - refPadHeight_ * dstStride;
            GetRefPixel(tmpSrc, tmpDst, srcStride, dstStride, refPadHeight_, puWidth_);
        } else {
            // padding up
            for (int i = 0; i < refPadHeight_; i++) {
                for (int j = 0; j < puWidth_; j++) {
                    dst[-refPadHeight_ * dstStride + j + i * dstStride] = dst[j];
                }
            }
        }

        //copy bottom
        if (!rowLastFlag) {
            tmpSrc = src + puHeight_ * srcStride;
            tmpDst = dst + puHeight_ * dstStride;
            GetRefPixel(tmpSrc, tmpDst, srcStride, dstStride, refPadHeight_, puWidth_);
        } else {
            // padding bottom
            for (int i = 0; i < refPadHeight_; i++) {
                for (int j = 0; j < puWidth_; j++) {
                    dst[puHeight_ * dstStride + j + i * dstStride] = dst[(puHeight_ - 1) * dstStride + j];
                }
            }
        }


        if (!colFirstFlag && !rowFirstFlag) {
            //copy left-top corner
            tmpSrc = src - refPadWidth_ - refPadHeight_ * srcStride;
            tmpDst = dst - refPadWidth_ - refPadHeight_ * dstStride;
            GetRefPixel(tmpSrc, tmpDst, srcStride, dstStride, refPadHeight_, refPadWidth_);
        } else {
            // padding left-top
            for (int i = 0; i < refPadHeight_; i++) {
                for (int j = 0; j < refPadWidth_; j++) {
                    if (colFirstFlag) {
                        dst[-refPadWidth_ - refPadHeight_ * dstStride + j + i * dstStride] = dst[-refPadHeight_ * dstStride + i * dstStride];
                    } else {
                        dst[-refPadWidth_ - refPadHeight_ * dstStride + j + i * dstStride] = dst[-refPadWidth_ + j];
                    }
                }
            }
        }

        if (!colLastFlag && !rowFirstFlag) {
            //copy right-top corner
            tmpSrc = src + puWidth_ - refPadHeight_ * srcStride;
            tmpDst = dst + puWidth_ - refPadHeight_ * dstStride;
            GetRefPixel(tmpSrc, tmpDst, srcStride, dstStride, refPadHeight_, refPadWidth_);
        } else {
            // padding right-top
            for (int i = 0; i < refPadHeight_; i++) {
                for (int j = 0; j < refPadWidth_; j++) {
                    if (colLastFlag) {
                        dst[puWidth_ - refPadHeight_ * dstStride + j + i * dstStride] = dst[-refPadHeight_ * dstStride + puWidth_ - 1 + i * dstStride];
                    } else {
                        dst[puWidth_ - refPadHeight_ * dstStride + j + i * dstStride] = dst[puWidth_ + j];
                    }
                }
            }
        }

        if (!colFirstFlag && !rowLastFlag) {
            //copy left-bottom corner
            tmpSrc = src - refPadWidth_ + puHeight_ * srcStride;
            tmpDst = dst - refPadWidth_ + puHeight_ * dstStride;
            GetRefPixel(tmpSrc, tmpDst, srcStride, dstStride, refPadHeight_, refPadWidth_);
        } else {
            // padding left-bottom
            for (int i = 0; i < refPadHeight_; i++) {
                for (int j = 0; j < refPadWidth_; j++) {
                    if (colFirstFlag)
                        dst[-refPadWidth_ + puHeight_ * dstStride + j + i * dstStride] = dst[puHeight_ * dstStride + i * dstStride];
                    else
                        dst[-refPadWidth_ + puHeight_ * dstStride + j + i * dstStride] = dst[-refPadWidth_ + j + (puHeight_ - 1) * dstStride];
                }
            }
        }

        if (!colLastFlag && !rowLastFlag) {
            //copy right-bottom corner
            tmpSrc = src + puWidth_ + puHeight_ * srcStride;
            tmpDst = dst + puWidth_ + puHeight_ * dstStride;
            GetRefPixel(tmpSrc, tmpDst, srcStride, dstStride, refPadHeight_, refPadWidth_);
        } else {
            // padding right-bottom
            for (int i = 0; i < refPadHeight_; i++) {
                for (int j = 0; j < refPadWidth_; j++) {
                    if (colLastFlag)
                        dst[puWidth_ + puHeight_ * dstStride + j + i * dstStride] = dst[puWidth_ - 1 + puHeight_ * dstStride + i * dstStride];
                    else
                        dst[puWidth_ + puHeight_ * dstStride + j + i * dstStride] = dst[puWidth_ + (puHeight_ - 1) * dstStride + j];
                }
            }
        }
    }

    void LLPuInter::GetRefPixel(Pel* src, Pel* dst, uint32_t srcStride, uint32_t dstStride, uint32_t height, uint32_t width) {
        for (uint32_t y = 0; y < height; y++) {
            for (uint32_t x = 0; x < width; x++) {
                dst[x] = src[x];
            }
            src += srcStride;
            dst += dstStride;
        }
    }


    void LLPuInter::InterPredInt(std::vector<int32_t>& interPred) {
        uint32_t srcIdx = 0;
        uint32_t dstIdx = refWinOrgIdx_;
        for (uint32_t yPos = 0; yPos < puHeight_; ++yPos) {
            for (uint32_t xPos = 0; xPos < puWidth_; ++xPos) {
                interPred[srcIdx + xPos] = ((*refWin_)[dstIdx + xPos] << DWT_SHIFT) + LL_SUB_BAND_ADD;
            }
            srcIdx += puWidth_;
            dstIdx += refWinStride_;
        }
    }

    void LLPuInter::InterPredHor(std::vector<int32_t>& interPred) {
        uint32_t srcIdx = 0;
        uint32_t dstIdx = refWinOrgIdx_;
        const int32_t tap0 = 1;
        const int32_t tap1 = 1;

        for (uint32_t yPos = 0; yPos < puHeight_; ++yPos) {
            for (uint32_t xPos = 0; xPos < puWidth_; ++xPos) {
                int32_t s0 = (*refWin_)[dstIdx + xPos] << DWT_SHIFT;
                int32_t s1 = (*refWin_)[dstIdx + xPos + 1] << DWT_SHIFT;
                interPred[srcIdx + xPos] = ((tap0 * s0 + tap1 * s1) >> 1) + LL_SUB_BAND_ADD;
            }
            srcIdx += puWidth_;
            dstIdx += refWinStride_;
        }
    }

    void LLPuInter::InterPredVer(std::vector<int32_t>& interPred) {
        uint32_t srcIdx = 0;
        uint32_t dstIdx = refWinOrgIdx_;
        const int32_t tap0 = 1;
        const int32_t tap1 = 1;

        for (uint32_t yPos = 0; yPos < puHeight_; ++yPos) {
            for (uint32_t xPos = 0; xPos < puWidth_; ++xPos) {
                int32_t s0 = (*refWin_)[dstIdx + xPos] << DWT_SHIFT;
                int32_t s1 = (*refWin_)[dstIdx + xPos + refWinStride_] << DWT_SHIFT;
                interPred[srcIdx + xPos] = ((tap0 * s0 + tap1 * s1) >> 1) + LL_SUB_BAND_ADD;
            }
            srcIdx += puWidth_;
            dstIdx += refWinStride_;
        }
    }

    void LLPuInter::InterPredHorVer(std::vector<int32_t>& interPred) {
        uint32_t srcIdx = 0;
        uint32_t dstIdx = refWinOrgIdx_;
        const int32_t tap0 = 1;
        const int32_t tap1 = 1;
        const int32_t tap2 = 1;
        const int32_t tap3 = 1;

        for (uint32_t yPos = 0; yPos < puHeight_; ++yPos) {
            for (uint32_t xPos = 0; xPos < puWidth_; ++xPos) {
                int32_t s0 = (*refWin_)[dstIdx + xPos] << DWT_SHIFT;
                int32_t s1 = (*refWin_)[dstIdx + xPos + 1] << DWT_SHIFT;
                int32_t s2 = (*refWin_)[dstIdx + xPos + refWinStride_] << DWT_SHIFT;
                int32_t s3 = (*refWin_)[dstIdx + xPos + refWinStride_ + 1] << DWT_SHIFT;
                interPred[srcIdx + xPos] = ((tap0 * s0 + tap1 * s1 + tap2 * s2 + tap3 * s3) >> 2) + LL_SUB_BAND_ADD;
            }
            srcIdx += puWidth_;
            dstIdx += refWinStride_;
        }
    }


    void LLPuInter::InterPred(MotionVector puMv, std::vector<int32_t>& interPred, uint32_t compId) {
        bool horFilter, verFilter;
        int32_t offsetX, offsetY;
        if (compId == Y) {
            horFilter = puMv.mvX % 2;
            verFilter = puMv.mvY % 2;
            offsetX = puMv.mvX >> 1;
            offsetY = puMv.mvY >> 1;
        } else {
            // for YUV422 format
            horFilter = puMv.mvX % 4;
            verFilter = puMv.mvY % 2;
            offsetX = puMv.mvX >> 2;
            offsetY = puMv.mvY >> 1;
        }

        refWinOrgIdx_ += (offsetX + offsetY * refWinStride_);
        if (!horFilter && !verFilter) {
            InterPredInt(interPred);
        } else if (horFilter && !verFilter) {
            InterPredHor(interPred);
        } else if (!horFilter && verFilter) {
            InterPredVer(interPred);
        } else {
            InterPredHorVer(interPred);
        }
    };
}

