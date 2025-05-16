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
#include "LLPuIntra.h"

namespace ProVivid {
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

    void LLPuIntra::IntraPredPlanar(std::vector<int32_t>& intraPred) {
        if (!leftAvail_ || !upAvail_) {
            SetDefault(intraPred);
            return;
        }
        uint8_t index = 0;
        for (uint8_t yPos = 0; yPos < puHeight_; ++yPos) {
            for (uint8_t xPos = 0; xPos < puWidth_; ++xPos) {
                intraPred[index + xPos] = ((*Nbr_[MB_NBR_LEFT])[yPos] + (*Nbr_[MB_NBR_UP])[xPos] + 1) >> 1;
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
        case (INTRA_PLANAR): {
            if (isChroma_ == LUMA) {
                IntraPredPlanar(intraPred);
            } else {
                //IntraPredCCLM(intraPred);
                exit(-1);
            }
            break;
        }
        default: {
            exit(-1);
        }
        }
    }

}

