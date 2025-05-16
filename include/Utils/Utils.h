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
#ifndef UTILS_H
#define UTILS_H

#include <array>
#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdint>
#include <string>
#include "Log.h"
#include "FrameBuffer.h"
#include "Table.h"

namespace HFM {

    typedef enum {
        PSNR_1020,
        PSNR_1023
    } PSNR_MODE;

    inline uint32_t Align(uint32_t size, uint32_t base) {
        return (size + base - 1) & ~(base - 1);
    }

    inline bool IsAllWhiteSpace(const std::string& str) {
        return std::all_of(str.begin(), str.end(), [](unsigned char c) {
            return std::isspace(c);
        });
    }

    inline std::string Trim(const std::string& s) {
        size_t start = 0;
        while (start < s.size() && isspace(s[start])) {
            start++;
        }
        if (start == s.size()) {
            return "";
        }
        size_t end = s.size() - 1;
        while (end >= start && isspace(s[end])) {
            end--;
        }
        return s.substr(start, end - start + 1);
    }

    inline std::vector<std::string> SplitString(std::string str, const std::string& pattern) {
        std::vector<std::string> result;
        str += pattern;
        auto size = str.size();
        decltype(size) pos;
        for (auto i = 0; i < size; i++) {
            pos = str.find(pattern, i);
            if (pos < size) {
                std::string s = str.substr(i, pos - i);
                result.push_back(s);
                i = pos + pattern.size() - 1;
            }
        }
        return result;
    }

    template <typename T1, typename T2>
    inline void Clip(const T1& in, T2& out, const T1& minValue, const T1& maxValue) {
        out = static_cast<T2>(std::max(minValue, std::min(maxValue, in)));
    }

    template <typename T1, typename T2>
    inline void Abs(const T1& in, T2& out) {
        out = abs(in);
    }

    template <typename T1>
    std::array<double, N_COLOR_COMP> CalcPSNR(T1* imgA, T1* imgB,
                                              std::array<ImgBufSize, LUMA_CHROMA>& sizeA,
                                              std::array<ImgBufSize, LUMA_CHROMA>& sizeB,
                                              uint32_t bitDepth, PSNR_MODE mode) {
        double peakVal = mode == PSNR_1020 ? 1020.0 : 1023.0;
        if (bitDepth == 8) {
            peakVal = 255.0;
        }

        T1* ptrA[N_COLOR_COMP] {nullptr};
        ptrA[Y] = imgA;
        ptrA[U] = ptrA[Y] + sizeA[LUMA].strideW * sizeA[LUMA].strideH;
        ptrA[V] = ptrA[U] + sizeA[CHROMA].strideW * sizeA[CHROMA].strideH;

        T1* ptrB[N_COLOR_COMP] {nullptr};
        ptrB[Y] = imgB;
        ptrB[U] = ptrB[Y] + sizeB[LUMA].strideW * sizeB[LUMA].strideH;
        ptrB[V] = ptrB[U] + sizeB[CHROMA].strideW * sizeB[CHROMA].strideH;

        if (sizeA[LUMA].w != sizeB[LUMA].w || sizeA[LUMA].h != sizeB[LUMA].h ||
            sizeA[CHROMA].w != sizeB[CHROMA].w || sizeA[CHROMA].h != sizeB[CHROMA].h) {
            LOGE("%s\n", "image A size != image B size");
        }

        double sum[N_COLOR_COMP] {0.0};
        double mse[N_COLOR_COMP] {0.0};
        std::array<double, N_COLOR_COMP> psnr {0.0};
        for (auto color : COLORS) {
            uint32_t width = color == Y ? sizeA[LUMA].w : sizeA[CHROMA].w;
            uint32_t height = color == Y ? sizeA[LUMA].h : sizeA[CHROMA].h;
            uint32_t strideA = color == Y ? sizeA[LUMA].strideW : sizeA[CHROMA].strideW;
            uint32_t strideB = color == Y ? sizeB[LUMA].strideW : sizeB[CHROMA].strideW;
            for (uint32_t h = 0; h < height; h++) {
                for (uint32_t w = 0; w < width; w++) {
                    auto diff = static_cast<double>(ptrA[color][w] - ptrB[color][w]);
                    sum[color] += diff * diff;
                }
                ptrA[color] += strideA;
                ptrB[color] += strideB;
            }
            mse[color] = sum[color] / (width * height);
            psnr[color] = (mse[color] < 1e-8) ? 100.0 : std::abs(10.0 * std::log10((peakVal * peakVal) / mse[color]));
        }
        return psnr;
    }

    inline float RdCostCal(uint32_t rate, uint32_t dis, uint32_t qp) {
        return (LAMBDA[qp] * static_cast<float>(rate) + static_cast<float>(dis));
    }
}

#endif // UTILS_H
