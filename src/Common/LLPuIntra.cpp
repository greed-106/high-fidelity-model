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
#include "LLPuIntra.h"

namespace HFM {
    LLPuIntra::LLPuIntra() {
        for (int i = MB_NBR_UP; i < N_NBR; i++) {
            Nbr_[i] = std::make_shared<PixelBuffer>(MB_SIZE, 0);
        }
    }
    
    LLPuIntra::~LLPuIntra() {
    }

    void LLPuIntra::Set(uint8_t puSize, uint8_t intraPredMode, uint32_t isChroma, uint8_t puX, uint8_t puY, uint8_t puWidth, uint8_t puHeight
        , uint32_t mbX, uint32_t mbY, uint32_t puPixelIndex, uint32_t lineWidth) {
        puSize_ = puSize;
        intraPredMode_ = intraPredMode;
        isChroma_ = isChroma;
        puWidth_ = puWidth;
        puHeight_ = puHeight;
        leftAvail_ = (puX != 0 || mbX != 0);
        upAvail_ = (puY != 0 || mbY != 0);
        puPixelIndex_ = puPixelIndex;
        lineWidth_ = lineWidth;
    }



    void LLPuIntra::GetPuNbr(SharedFrameBuffer llBandPicRec) {
        uint32_t index = puPixelIndex_;
        if (leftAvail_) {
            for (uint8_t leftPos = 0; leftPos < puHeight_; leftPos++) {
                (*Nbr_[MB_NBR_LEFT])[leftPos] = (*llBandPicRec)[index - 1 + leftPos * lineWidth_];
            }
        }
        if (upAvail_) {
            for (uint8_t upPos = 0; upPos < puWidth_; upPos++) {
                (*Nbr_[MB_NBR_UP])[upPos] = (*llBandPicRec)[index - lineWidth_ + upPos];
            }
        }
    }


    void LLPuIntra::SetDefault(std::vector<int32_t>& intraPred) {
        for (uint8_t index = 0; index < puWidth_*puHeight_; index++) {
            intraPred[index] = INTRA_DEFAULT;
        }
    }

    void LLPuIntra::IntraPredVer(std::vector<int32_t>& intraPred) {
        if (!upAvail_) {
            SetDefault(intraPred);
            return;
        }
        uint8_t index = 0;
        for (uint8_t yPos = 0; yPos < puHeight_; ++yPos) {
            for (uint8_t xPos = 0; xPos < puWidth_; ++xPos) {
                intraPred[index + xPos] = ((*Nbr_[MB_NBR_UP])[xPos]);
            }
            index += puWidth_;
        }
    }

    void LLPuIntra::IntraPredDC(std::vector<int32_t>& intraPred) {
        if (!upAvail_ && !leftAvail_) {
            SetDefault(intraPred);
            return;
        }
        uint8_t index = 0;
        uint8_t xPos, yPos;
        int32_t dcVal = 0;
        int32_t shift = 0;
        int32_t offset = 0;
        if (upAvail_) {
            if (puWidth_ == puHeight_ && leftAvail_) {
                for (xPos = 0; xPos < puWidth_ >> 1; ++xPos) {
                    dcVal += (*Nbr_[MB_NBR_UP])[xPos * 2 + 1];
                }
            } else {
                for (xPos = 0; xPos < puWidth_; ++xPos) {
                    dcVal += (*Nbr_[MB_NBR_UP])[xPos];
                }
            }
        }
        if (leftAvail_) {
            if (upAvail_) {
                for (yPos = 0; yPos < puHeight_ >> 1; ++yPos) {
                    dcVal += (*Nbr_[MB_NBR_LEFT])[yPos * 2 + 1];
                }
            } else {
                for (yPos = 0; yPos < puHeight_; ++yPos) {
                    dcVal += (*Nbr_[MB_NBR_LEFT])[yPos];
                }
            }
        }
        if ((puWidth_ == 4 && puHeight_ == 4) || ((!leftAvail_) && puWidth_ == 4)) {
            shift = 2;
        } else {
            shift = 3;
        }
        offset = (1 << (shift - 1));
        dcVal = (dcVal + offset) >> shift;
        for (uint8_t yPos = 0; yPos < puHeight_; ++yPos) {
            for (uint8_t xPos = 0; xPos < puWidth_; ++xPos) {
                intraPred[index + xPos] = dcVal;
            }
            index += puWidth_;
        }
    }

    void LLPuIntra::IntraPredHor(std::vector<int32_t>& intraPred) {
        if (!leftAvail_) {
            SetDefault(intraPred);
            return;
        }
        uint8_t index = 0;
        for (uint8_t yPos = 0; yPos < puHeight_; ++yPos) {
            for (uint8_t xPos = 0; xPos < puWidth_; ++xPos) {
                intraPred[index + xPos] = ((*Nbr_[MB_NBR_LEFT])[yPos]);
            }
            index += puWidth_;
        }
    }

    void LLPuIntra::ComLLPred(std::vector<int32_t>& intraPred) {
        switch (intraPredMode_) {
        case (INTRA_VER): {
            IntraPredVer(intraPred);
            break;
        }
        case (INTRA_DC): {
            IntraPredDC(intraPred);
            break;
        }
        case (INTRA_HOR): {
            IntraPredHor(intraPred);
            break;
        }
        default: {
            exit(-1);
        }
        }
    }

}

