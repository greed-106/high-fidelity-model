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
#include <string>
#include <utility>
#include "Const.h"
#include "Log.h"
#include "SubPicEnc.h"
#include "Timer.h"
#include "Utils.h"
#include "Wavelet.h"

namespace HFM {
    SubPicEnc::SubPicEnc(PixelFormat pixelFormat, uint32_t picWidth, uint32_t picHeight, uint32_t subPicWidth, uint32_t subPicHeight) {
        SubPic::Init(pixelFormat, picWidth, picHeight, subPicWidth, subPicHeight);
        subBandPixels_[Y] = (subPicSize_[LUMA].strideW * subPicSize_[LUMA].strideH * 2) >> 2;
        subBandPixels_[U] = (subPicSize_[CHROMA].strideW * subPicSize_[CHROMA].strideH * 2) >> 2;
        subBandPixels_[V] = subBandPixels_[U];
        for (const auto & band : SUB_BANDS) {
            for (const auto & subBandPixel : subBandPixels_) {
                subBands_[band].emplace_back(std::make_shared<FrameBuffer>(subBandPixel, 0));
                subBandsRec_[band].emplace_back(std::make_shared<FrameBuffer>(subBandPixel, 0));
                if (band == LL) {
                  subBandsRef_[band].emplace_back(std::make_shared<FrameBuffer>(subBandPixel, 0));
                }
            }
        }
        alphaBuffer_ = std::make_shared<BufferStorage>(subPicSize_[LUMA].strideW * subPicSize_[LUMA].strideH * 2, 0);
        dwtRowBuffer_ = std::make_shared<FrameBuffer>(subPicSize_[LUMA].strideW, 0);
        dwtTransTmpBuffer_ = std::make_shared<FrameBuffer>(subPicSize_[LUMA].strideW * subPicSize_[LUMA].strideH * 2, 0);
    }

    void SubPicEnc::DWT(SubPicInfoMap& subPicInfo) {
        //Timer timer("Encoder::" + std::string(__FUNCTION__));
        //std::string dwtDesc{"dwt for sub-pic: " + std::to_string(subPicInfo[Y].id)};
        //timer.Start(dwtDesc);
        // Md5Printer md5Printer;
        for (const auto & color : YUVS) {
            auto info = subPicInfo[color];
            auto currPos = reinterpret_cast<PelStorage*>(info.picHeaderPtr) + info.y * info.strideW + info.x;
            // hor-trans
            Pel* horBufL = dwtTransTmpBuffer_->data();
            Pel* horBufH = horBufL + (info.w >> 1);
            for (int h = 0; h < info.h; ++h) {
                for (int w = 0; w < info.w; ++w) {
                    dwtRowBuffer_->at(w) = static_cast<Pel>(currPos[w]) << DWT_SHIFT;
                }
                if (color == Y) {
                    IntConvDWT97(dwtRowBuffer_->data(), info.w, horBufL, horBufH);
                } else {
                    DWT53(dwtRowBuffer_->data(), info.w, horBufL, horBufH);
                }
                currPos += info.strideW;
                horBufL += info.w;
                horBufH += info.w;
            }
            // ver-trans
            horBufL = dwtTransTmpBuffer_->data();
            horBufH = horBufL + (info.w >> 1);
            Pel* subBandLL = subBands_[LL][color]->data();
            Pel* subBandLH = subBands_[LH][color]->data();
            Pel* subBandHL = subBands_[HL][color]->data();
            Pel* subBandHH = subBands_[HH][color]->data();
            int horLen = static_cast<int>(info.w);
            int verLen = static_cast<int>(info.h);
            int subBandHorLen = horLen >> 1;
            for (int w = 0; w < subBandHorLen; ++w) {
                DWT53(horBufL, verLen, subBandLL, subBandLH, horLen, subBandHorLen);
                DWT53(horBufH, verLen, subBandHL, subBandHH, horLen, subBandHorLen);
                horBufL++;
                horBufH++;
                subBandLL++;
                subBandLH++;
                subBandHL++;
                subBandHH++;
            }
            subBandLL = subBands_[LL][color]->data();
            uint32_t subBandPixels = (info.w * info.h) >> 2;
            for (uint32_t i = 0; i < subBandPixels; ++i) {
                *subBandLL += LL_SUB_BAND_ADD;
                Clip(*subBandLL, *subBandLL, LL_SUB_BAND_MIN, LL_SUB_BAND_MAX);
                subBandLL++;
            }
            /*
            for (auto band : SUB_BANDS) {
                md5Printer.GetMd5((char *)subBands_[band][color]->data(), subBandPixels * sizeof(Pel), "sub-band " + SUB_BANDS_STR[band] + " " + COLORS_STR[color]);
            }
             */
        }
        //timer.End(dwtDesc);
        if (!dwtSubPicPath_.empty()) {
            WriteDWTSubPic(subPicInfo, dwtSubPicPath_);
        }
    }

    void SubPicEnc::GetAlpha(SubPicInfoMap& subPicInfo) {
            auto info = subPicInfo[A];
            auto currPos = reinterpret_cast<PelStorage*>(info.picHeaderPtr) + info.y * info.strideW + info.x;
            PelStorage* alphaPtr = alphaBuffer_->data();
            for (int h = 0; h < info.h; ++h) {
                for (int w = 0; w < info.w; ++w) {
                    alphaPtr[w] = currPos[w];
                }
                currPos += info.strideW;
                alphaPtr += info.w;
            }
    }

    void SubPicEnc::SetLLReference(SubPicInfoMap& subPicLLInfoRef) {
        for (const auto & color : YUVS) {
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


    void SubPicEnc::SetDWTSubPicPath(std::string dwtSubPicPath) {
        dwtSubPicPath_ = std::move(dwtSubPicPath);
    }

    void SubPicEnc::WriteDWTSubPic(SubPicInfoMap& subPicInfo, const std::string& path) {
        std::ofstream dwtFile;
        std::string wStr = std::to_string(subPicInfo[Y].w);
        std::string hStr = std::to_string(subPicInfo[Y].h);
        std::string idStr = std::to_string(subPicInfo[Y].id);
        dwtFile.open(path + "/dwt_subpic_" + idStr + "_" + wStr + "x" + hStr + "_422_16bits.yuv", std::ofstream::binary | std::ofstream::trunc);
        for (auto & color : YUVS) {
            PelStorage element;
            uint32_t subBandHeight = subPicInfo[color].h >> 1;
            uint32_t subBandWidth = subPicInfo[color].w >> 1;
            Pel* subBandLL = subBands_[LL][color]->data();
            Pel* subBandLH = subBands_[LH][color]->data();
            Pel* subBandHL = subBands_[HL][color]->data();
            Pel* subBandHH = subBands_[HH][color]->data();
            for (int h = 0; h < subBandHeight; ++h) {
                for (int w = 0; w < subBandWidth; ++w) {
                    element = static_cast<PelStorage>(*subBandLL) + DWT_SUB_PIC_OFFSET;
                    dwtFile.write((const char*)&element, sizeof(PelStorage));
                    subBandLL++;
                }
                for (int w = 0; w < subBandWidth; ++w) {
                    element = static_cast<PelStorage>(*subBandHL) + DWT_SUB_PIC_OFFSET;
                    dwtFile.write((const char*)&element, sizeof(PelStorage));
                    subBandHL++;
                }
            }
            for (int h = 0; h < subBandHeight; ++h) {
                for (int w = 0; w < subBandWidth; ++w) {
                    element = static_cast<PelStorage>(*subBandLH) + DWT_SUB_PIC_OFFSET;
                    dwtFile.write((const char*)&element, sizeof(PelStorage));
                    subBandLH++;
                }
                for (int w = 0; w < subBandWidth; ++w) {
                    element = static_cast<PelStorage>(*subBandHH) + DWT_SUB_PIC_OFFSET;
                    dwtFile.write((const char*)&element, sizeof(PelStorage));
                    subBandHH++;
                }
            }
        }
        dwtFile.close();
    }
}
