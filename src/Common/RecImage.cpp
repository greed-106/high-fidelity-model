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
#include "Log.h"
#include "RecImage.h"
#include "Status.h"
#include "Utils.h"
#include "Wavelet.h"
#include <string.h>

namespace ProVivid {
    Status WriteRecPic(const SharedBufferStorage& recPic,
                       const std::vector<std::pair<uint32_t, uint32_t>>& size,
                       std::ofstream& recFileHandle) {
        if (!recFileHandle.is_open()) {
            auto status = Status::INVALID_FILE_HANDLE;
            LOGE("%s\n", GetStatusMsg(status));
            return status;
        }
        std::string wStr = std::to_string(size[Y].first);
        std::string hStr = std::to_string(size[Y].second);
        auto recPicPtr = recPic->data();
        for (auto & color : COLORS) {
            uint32_t pixels = size[color].first * size[color].second;
            recFileHandle.write((const char*)recPicPtr, pixels * sizeof(PelStorage));
            recPicPtr += pixels;
        }
        return Status::SUCCESS;
    }

    std::pair<uint32_t, uint32_t> GetRecSubPicSize(SubPicInfo& info, std::pair<uint32_t, uint32_t> size) {
        uint32_t subPicW = info.w;
        uint32_t subPicH = info.h;
        uint32_t subPicPosX = info.x;
        uint32_t subPicPosY = info.y;
        uint32_t recSubPicWidth = std::min(subPicW, size.first - subPicPosX);
        uint32_t recSubPicHeight = std::min(subPicH, size.second - subPicPosY);
        return {recSubPicWidth, recSubPicHeight};
    }

    void RecLLSubPic(SubPicInfoMap& subPicInfo, const std::vector<SharedFrameBuffer>& subBandLL,
        std::vector<std::pair<uint32_t, uint32_t>> recSubPicLLSize, uint32_t bitDepth) {
        int32_t maxValue = (1 << bitDepth) - 1;
        for (auto color : COLORS) {
            auto info = subPicInfo[color];
            Pel* subBandLLPtr = subBandLL[color]->data();
            auto recLLPtr = reinterpret_cast<PelStorage*>(info.picHeaderPtr) + info.y * info.strideW + info.x;
            for (int h = 0; h < recSubPicLLSize[color].second; ++h) {
                for (int w = 0; w < recSubPicLLSize[color].first; ++w) {
                    Pel recPixel = (subBandLLPtr[w] + (1 << (DWT_SHIFT - 1))) >> DWT_SHIFT;
                    Clip(recPixel, recLLPtr[w], 0, maxValue);
                }
                subBandLLPtr += info.w;
                recLLPtr += info.strideW;
            }
        }
    }

    void RefLLSubPic(SubPicInfoMap& subPicInfo, const std::vector<SharedFrameBuffer>& subBandLL,
        std::vector<std::pair<uint32_t, uint32_t>> recSubPicLLSize, std::vector<std::pair<uint32_t, uint32_t>> refSubPicLLSize, uint32_t bitDepth) {
        int32_t maxValue = (1 << bitDepth) - 1;
        for (auto color : COLORS) {
            auto info = subPicInfo[color];
            Pel* subBandLLPtr = subBandLL[color]->data();
            auto recLLPtr = reinterpret_cast<PelStorage*>(info.picHeaderPtr) + info.y * info.strideW + info.x;
            for (uint32_t h = 0; h < recSubPicLLSize[color].second; ++h) {
                for (uint32_t w = 0; w < recSubPicLLSize[color].first; ++w) {
                    Pel recPixel = (subBandLLPtr[std::min(w, refSubPicLLSize[color].first - 1)] + (1 << (DWT_SHIFT - 1))) >> DWT_SHIFT;
                    Clip(recPixel, recLLPtr[w], 0, maxValue);
                }
                if(h < refSubPicLLSize[color].second - 1) 
                    subBandLLPtr += info.w;
                recLLPtr += info.strideW;
            }
            
        }
    }

    Status RecImage(SubBandMap& subBands,
                    SubPicInfoMap& subPicInfo,
                    std::vector<SubPicInfoMap>& subPicInfoRec,
                    std::vector<SubPicInfoMap>& subPicLLInfoRec,
                    std::vector<SubPicInfoMap>& subPicLLInfoRef,
                    const SharedFrameBuffer& tmpRowBuffer,
                    const SharedFrameBuffer& tmpPicBuffer,
                    const std::vector<std::pair<uint32_t, uint32_t>>& picSize,
                    const std::vector<std::pair<uint32_t, uint32_t>>& picLLSize,
                    const std::string& recFile, const std::string& recLLFile, uint32_t bitDepth, bool needRef) {
        uint32_t subPicId = subPicInfo[Y].id;
        if (!recFile.empty() || !recLLFile.empty() || needRef) {
            for (auto color : COLORS) {
                for (auto & v : *subBands[LL][color]) {
                    v -= LL_SUB_BAND_ADD;
                }
            }
            if (!recFile.empty()) {
                if (subPicInfoRec.size() <= subPicId) {
                    Status status = Status::INVALID_SUB_PIC_INFO;
                    LOGE("subPicInfoRec: %s\n", GetStatusMsg(status));
                    return status;
                }
                std::vector<std::pair<uint32_t, uint32_t>> recSubPicSize;
                for (auto color : COLORS) {
                    recSubPicSize.emplace_back(GetRecSubPicSize(subPicInfo[color], picSize[color]));
                }
                IDWT(subPicInfoRec[subPicId], subBands, tmpPicBuffer, tmpRowBuffer, recSubPicSize, bitDepth);
            }
            if (!recLLFile.empty()) {
                if (subPicLLInfoRec.size() <= subPicId) {
                    Status status = Status::INVALID_SUB_PIC_INFO;
                    LOGE("subPicLLInfoRec: %s\n", GetStatusMsg(status));
                    return status;
                }
                std::vector<std::pair<uint32_t, uint32_t>> recSubPicLLSize;
                for (auto color : COLORS) {
                    recSubPicLLSize.emplace_back(GetRecSubPicSize(subPicLLInfoRec[subPicId][color], picLLSize[color]));
                }
                RecLLSubPic(subPicLLInfoRec[subPicId], subBands[LL], recSubPicLLSize, bitDepth);
            }
            if (needRef) { 
                std::vector<std::pair<uint32_t, uint32_t>> refSubPicLLSize;
                std::vector<std::pair<uint32_t, uint32_t>> recSubPicLLSize;
                for (auto color : COLORS) {
                    auto info = subPicLLInfoRef[subPicId][color];
                    recSubPicLLSize.emplace_back(info.w, info.h);
                    refSubPicLLSize.emplace_back(GetRecSubPicSize(subPicLLInfoRef[subPicId][color], picLLSize[color]));
                }
                RefLLSubPic(subPicLLInfoRef[subPicId], subBands[LL], recSubPicLLSize, refSubPicLLSize, bitDepth);
            }
        }
        return Status::SUCCESS;
    }
}
