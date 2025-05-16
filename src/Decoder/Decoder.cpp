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

//#include <algorithm>
#include <cstdint>
#include <fstream>
#include <string>
#include <iostream>
//#include <utility>
#include "Decoder.h"
#include "Log.h"
#include <sys/stat.h>
#include "RecImage.h"
//#include "Wavelet.h"
//#include "context_ini.h"
//#include "biariencode.h"
//#include "cabac.h"
//
namespace ProVivid {
    Decoder::Decoder(uint32_t bitDepth, std::string decFile, std::string decLLFile, const std::string& bitstreamFile, uint32_t intraPeriod)
        : bitDepth_(bitDepth), decFile_(std::move(decFile)), decLLFile_(std::move(decLLFile)) {
        seqPicHeaderInfo_.frameCount = 1;
        //bitstream initialization
        bitstream_ = { 0 };
        std::ifstream inF;
        inF.open(bitstreamFile, std::ifstream::binary);
        if (inF.fail()) {
            LOGE("failed to open: %s\n", bitstreamFile.c_str());
        }
        inF.seekg(0, std::ios::end);
        int64_t fileSize = inF.tellg();
        inF.seekg(0, std::ios::beg);
        bitstream_.streamBuffer = new byte[fileSize];
        inF.read(reinterpret_cast<char*>(bitstream_.streamBuffer), fileSize);
        if (inF.is_open()) {
            inF.close();
        }

        intraPeriod_ = intraPeriod;

        bitstream_.bitstream_length = fileSize;
        curBitstreamPos_ = 0;
        bitstreamLength_ = fileSize;

        if (!decFile_.empty()) {
            decFileHandle_.open(decFile_, std::ofstream::binary | std::ofstream::trunc);
            if (decFileHandle_.fail()) {
                LOGE("failed to open: %s\n", decFile_.c_str());
            } else {
                LOGI("open: %s\n", decFile_.c_str());
            }
        }

        if (!decLLFile_.empty()) {
            decLLFileHandle_.open(decLLFile_, std::ofstream::binary | std::ofstream::trunc);
            if (decLLFileHandle_.fail()) {
                LOGE("failed to open: %s\n", decLLFile_.c_str());
            } else {
                LOGI("open: %s\n", decLLFile_.c_str());
            }
        }
    }

    Decoder::~Decoder() {
        if (decFileHandle_.is_open()) {
            decFileHandle_.close();
        }
        if (decLLFileHandle_.is_open()) {
            decLLFileHandle_.close();
        }
    }

    void Decoder::RenderingInformation(Bitstream* bitstream) {
        int cicpInfoPresentFlag = read_u_v(1, bitstream);
        int mdcvInfoPresentFlag = read_u_v(1, bitstream);
        int clliInfoPresentFlag = read_u_v(1, bitstream);
        int dmPresentFlag = read_u_v(1, bitstream);
        int reservedBits = read_u_v(4, bitstream);

        if (cicpInfoPresentFlag) {
            int colourPrimaries = read_u_v(8, bitstream);
            int transferCharacteristics = read_u_v(8, bitstream);
            int matrixCoefficients = read_u_v(8, bitstream);
            int imageFullRangeFlag = read_u_v(1, bitstream);
            int chroma420SampleLocType = read_u_v(3, bitstream);
            reservedBits = read_u_v(4, bitstream);
        }
        if (mdcvInfoPresentFlag) {
            int masteringDisplayColourPrimariesX[3] = { 0 };
            int masteringDisplayColourPrimariesY[3] = { 0 };
            for (int i = 0; i < 3; i++) {
                masteringDisplayColourPrimariesX[i] = read_u_v(16, bitstream);
                masteringDisplayColourPrimariesY[i] = read_u_v(16, bitstream);
            }
            int masteringDisplayWhitePointChromaticityX = read_u_v(16, bitstream);
            int masteringDisplayWhitePointChromaticityY = read_u_v(16, bitstream);
            int masteringDisplayMaximumLuminance = read_u_v(32, bitstream);
            int masteringDisplayMinimumLuminance = read_u_v(32, bitstream);
        }
        if (clliInfoPresentFlag) {
            int maximumContentLightLevel = read_u_v(16, bitstream);
            int maximumFrameAverageLightLevel = read_u_v(16, bitstream);
        }
        if (dmPresentFlag) {
            int dmType = read_u_v(8, bitstream);
            int dmSize = read_u_v(16, bitstream);
            int dmDataByte[65536] = { 0 };
            for (int i = 0; i < dmSize; i++) {
                dmDataByte[i] = read_u_v(8, bitstream);
            }
        }
    }


    void Decoder::ParseSeqHeaderInfo(Bitstream* bitstream) {
        seqPicHeaderInfo_.profileIdc = read_u_v(8, bitstream);
        seqPicHeaderInfo_.levelIdc = read_u_v(8, bitstream);
        seqPicHeaderInfo_.frameCount = read_u_v(2, bitstream) + 1;
        seqPicHeaderInfo_.frameRate = read_u_v(8, bitstream);
        seqPicHeaderInfo_.width = read_u_v(16, bitstream);
        seqPicHeaderInfo_.height = read_u_v(16, bitstream);
        seqPicHeaderInfo_.subPicWidth = (read_u_v(3, bitstream) + 2) << 7;
        seqPicHeaderInfo_.subPicHeight = (read_u_v(6, bitstream) + 2) << 7;

        if (seqPicHeaderInfo_.subPicWidth > 1024 || seqPicHeaderInfo_.subPicWidth < 256 || seqPicHeaderInfo_.subPicWidth % 128 != 0) {
            printf("sub pic width shall be in range of 256 to 1024 and multiple of 128\n");
            exit(-1);
        }
        if (seqPicHeaderInfo_.subPicHeight > 4352 || seqPicHeaderInfo_.subPicHeight < 256 || seqPicHeaderInfo_.subPicHeight % 128 != 0) {
            printf("sub pic height shall be in range of 256 to 4352 and multiple of 128\n");
            exit(-1);
        }

        if (seqPicHeaderInfo_.subPicWidth > ((seqPicHeaderInfo_.width + 15) / 16) * 16)
            seqPicHeaderInfo_.subPicWidth = ((seqPicHeaderInfo_.width + 15) / 16) * 16;
        if (seqPicHeaderInfo_.subPicHeight > ((seqPicHeaderInfo_.height + 15) / 16) * 16)
            seqPicHeaderInfo_.subPicHeight = ((seqPicHeaderInfo_.height + 15) / 16) * 16;

        seqPicHeaderInfo_.bitDepth = read_u_v(4, bitstream) + 8;
        seqPicHeaderInfo_.pixelFormat = read_u_v(2, bitstream); //0: YUV444, 1: YUV422, 2: YUV420

        RenderingInformation(bitstream);
        int reservedBits = read_u_v(23, bitstream);

        //set sub info
        int subNumHor = (seqPicHeaderInfo_.width + seqPicHeaderInfo_.subPicWidth - 1) / seqPicHeaderInfo_.subPicWidth;
        int subNumVer = (seqPicHeaderInfo_.height + seqPicHeaderInfo_.subPicHeight - 1) / seqPicHeaderInfo_.subPicHeight;
        seqPicHeaderInfo_.numOfSubpic = subNumHor * subNumVer;
        subpicSyntaxInfo_.resize(seqPicHeaderInfo_.numOfSubpic);
        int rightSubpicWidth = ((seqPicHeaderInfo_.width + 15) / 16) * 16 - seqPicHeaderInfo_.subPicWidth * (subNumHor - 1);
        int bottomSubpicHeight = ((seqPicHeaderInfo_.height + 15) / 16) * 16 - seqPicHeaderInfo_.subPicHeight * (subNumVer - 1);
        for (int i = 0; i < subNumHor; i++) {
            for (int j = 0; j < subNumVer; j++) {
                subpicSyntaxInfo_[j * subNumHor + i].subpicWidth = (i == subNumHor - 1 ? rightSubpicWidth : seqPicHeaderInfo_.subPicWidth);
                subpicSyntaxInfo_[j * subNumHor + i].subpicHeight = (j == subNumVer - 1 ? bottomSubpicHeight : seqPicHeaderInfo_.subPicHeight);
            }
        }

    }

    void Decoder::ParsePicHeaderInfo(Bitstream* bitstream) {
        seqPicHeaderInfo_.picSize = read_u_v(32, bitstream); //pic length
        seqPicHeaderInfo_.frameType = read_u_v(1, bitstream); //0: I, 1: P
        seqPicHeaderInfo_.qpDeltaEnable = read_u_v(1, bitstream);
        seqPicHeaderInfo_.hfTransformSkip = read_u_v(1, bitstream);
        int reservedBits = read_u_v(21, bitstream);

        if (seqPicHeaderInfo_.width > 4096) {
            if (seqPicHeaderInfo_.numOfSubpic > 32) {
                printf("number of subpics shall be no more than 32 for image width larger than 4096\n");
                exit(-1);
            }
        } else if (seqPicHeaderInfo_.width > 2048) {
            if (seqPicHeaderInfo_.numOfSubpic > 16) {
                printf("number of subpics shall be no more than 16 for image width larger than 2048\n");
                exit(-1);
            }
        } else {
            if (seqPicHeaderInfo_.numOfSubpic > 8) {
                printf("number of subpics shall be no more than 8 for others\n");
                exit(-1);
            }
        }

        for (int i = 0; i < seqPicHeaderInfo_.numOfSubpic; i++) {
            subpicSyntaxInfo_[i].subpicLlQpIndex = read_u_v(6, bitstream);
            subpicSyntaxInfo_[i].subpicHlQpIndexOffset = read_u_v(5, bitstream) - 12;
            subpicSyntaxInfo_[i].subpicLhQpIndexOffset = read_u_v(5, bitstream) - 12;
            subpicSyntaxInfo_[i].subpicHhQpIndexOffset = read_u_v(5, bitstream) - 12;
            subpicSyntaxInfo_[i].subpicCbQpIndexOffset = read_u_v(5, bitstream) - 12;
            subpicSyntaxInfo_[i].subpicCrQpIndexOffset = read_u_v(5, bitstream) - 12;
            reservedBits = read_u_v(9, bitstream);
            subpicSyntaxInfo_[i].subpicLength = read_u_v(24, bitstream);
            subpicSyntaxInfo_[i].subpicLlCabacLength = read_u_v(24, bitstream);
            subpicSyntaxInfo_[i].subpicLlVlcLength = read_u_v(24, bitstream);
            subpicSyntaxInfo_[i].subpicHfCabacLength = read_u_v(24, bitstream);
        }
    }

    void Decoder::ParseSeqPicHeaderInfo(int frameIdx, Bitstream* bitstream) {
        if (frameIdx % seqPicHeaderInfo_.frameCount == 0) {
            ParseSeqHeaderInfo(bitstream);
        }
        ParsePicHeaderInfo(bitstream);
        curBitstreamPos_ += bitstream_.frame_bitoffset >> 3;
    }

    void Decoder::SetSubPic(std::shared_ptr<SubPicDec> subPicDec) {
        subPicDec_ = std::move(subPicDec);
        if (!decFile_.empty() && !decPicBuffer_) {
            subPicInfoDec_ = subPicDec_->subPicInfo_;
            uint32_t picPixels[N_COLOR_COMP] {};
            for (auto color : COLORS) {
                auto comp = color == Y ? LUMA : CHROMA;
                recPicSize_[comp] = subPicDec_->GetPicSizeRaw(color);
                // rec image size is W x H, stored in W x H buffer
                recPicSize_[comp].strideW = recPicSize_[comp].w;
                recPicSize_[comp].strideH = recPicSize_[comp].h;
                picPixels[color] += recPicSize_[comp].strideW * recPicSize_[comp].strideH;
                for (auto & info : subPicInfoDec_) {
                    info[color].strideW = recPicSize_[comp].w;
                    info[color].strideH = recPicSize_[comp].h;
                }
                picWHRaw_.emplace_back(recPicSize_[comp].w, recPicSize_[comp].h);
            }
            if (!decPicBuffer_) {
                decPicBuffer_ = std::make_shared<BufferStorage>(picPixels[Y] + picPixels[U] + picPixels[V], 0);
                PelStorage* picHeaderPtr[N_COLOR_COMP] {};
                picHeaderPtr[Y] = decPicBuffer_->data();
                picHeaderPtr[U] = picHeaderPtr[Y] + picPixels[Y];
                picHeaderPtr[V] = picHeaderPtr[U] + picPixels[U];
                for (auto & info : subPicInfoDec_) {
                    for (auto color: COLORS) {
                        info[color].picHeaderPtr = picHeaderPtr[color];
                    }
                }
            }
        }
        {
            subPicLLInfoDec_ = subPicDec_->subPicInfo_;
            uint32_t picLLPixels[N_COLOR_COMP] {};
            for (auto color : COLORS) {
                auto picSize = subPicDec_->GetPicSizeRaw(color);
                // low pass sub-band element count is (inputLen + 1) >> 1
                uint32_t widthLL = (picSize.w + 1) >> 1;
                uint32_t heightLL = (picSize.h + 1) >> 1;
                picLLPixels[color] += widthLL * heightLL;
                for (auto & info : subPicLLInfoDec_) {
                    info[color].w >>= 1;
                    info[color].h >>= 1;
                    info[color].x >>= 1;
                    info[color].y >>= 1;
                    info[color].strideW = widthLL;
                    info[color].strideH = heightLL;
                }
                picWHRawLL_.emplace_back(widthLL, heightLL);
            }
            if (!decLLFile_.empty() && !decPicLLBuffer_) {

                if (!decPicLLBuffer_) {
                    decPicLLBuffer_ = std::make_shared<BufferStorage>(picLLPixels[Y] + picLLPixels[U] + picLLPixels[V], 0);
                    PelStorage* picLLHeaderPtr[N_COLOR_COMP] {};
                    picLLHeaderPtr[Y] = decPicLLBuffer_->data();
                    picLLHeaderPtr[U] = picLLHeaderPtr[Y] + picLLPixels[Y];
                    picLLHeaderPtr[V] = picLLHeaderPtr[U] + picLLPixels[U];
                    for (auto & info : subPicLLInfoDec_) {
                        for (auto color: COLORS) {
                            info[color].picHeaderPtr = picLLHeaderPtr[color];
                        }
                    }
                }
            }
        }
        {
            if (!refPicLLBuffer_) {
                subPicLLInfoRef_ = subPicDec_->subPicInfo_;
                uint32_t picLLPixels[N_COLOR_COMP] {};
                for (auto color : COLORS) {
                    auto picSize = subPicDec_->GetPicSizeRaw(color);
                    uint32_t widthLL = (picSize.strideW + 1) >> 1;
                    uint32_t heightLL = (picSize.strideH + 1) >> 1;
                    picLLPixels[color] += widthLL * heightLL;
                    for (auto & info : subPicLLInfoRef_) {
                        info[color].w >>= 1;
                        info[color].h >>= 1;
                        info[color].x >>= 1;
                        info[color].y >>= 1;
                        info[color].strideW = widthLL;
                        info[color].strideH = heightLL;
                    }
                    //picLLSize_.emplace_back(widthLL, heightLL);
                }
                refPicLLBuffer_ = std::make_shared<BufferStorage>(picLLPixels[Y] + picLLPixels[U] + picLLPixels[V], 0);
                PelStorage* picLLHeaderPtr[N_COLOR_COMP] {};
                picLLHeaderPtr[Y] = refPicLLBuffer_->data();
                picLLHeaderPtr[U] = picLLHeaderPtr[Y] + picLLPixels[Y];
                picLLHeaderPtr[V] = picLLHeaderPtr[U] + picLLPixels[U];
                for (auto & info : subPicLLInfoRef_) {
                    for (auto color: COLORS) {
                        info[color].picHeaderPtr = picLLHeaderPtr[color];
                    }
                }
            }
        }
    }

    void Decoder::Decode(uint32_t currFrame) {
        auto frameType = static_cast<FrameType>(currFrame % intraPeriod_);
        bool needRef = frameType == FRAME_I && intraPeriod_ > 1;
        auto llDecoder = std::make_shared<LLDecoder>();
        auto hfDecoder = std::make_shared<HFDecoder>();
        for (int subPicIndex = 0; subPicIndex < seqPicHeaderInfo_.numOfSubpic; subPicIndex++) {
            Clip<uint32_t, uint32_t>(subpicSyntaxInfo_[subPicIndex].subpicLlQpIndex, qp_, 0, 39);
            if (frameType == FRAME_P) {
                subPicDec_->SetLLReference(subPicLLInfoRef_[subPicIndex]);
                llDecoder->Set(subpicSyntaxInfo_[subPicIndex].subpicWidth >> 1, subpicSyntaxInfo_[subPicIndex].subpicHeight >> 1,
                    subPicDec_->subBands_[LL], subPicDec_->subBandsRef_[LL],
                    qp_, subpicSyntaxInfo_[subPicIndex].subpicCbQpIndexOffset, subpicSyntaxInfo_[subPicIndex].subpicCrQpIndexOffset);
            } else {
                llDecoder->Set(subpicSyntaxInfo_[subPicIndex].subpicWidth >> 1, subpicSyntaxInfo_[subPicIndex].subpicHeight >> 1,
                    subPicDec_->subBands_[LL], 
                    qp_, subpicSyntaxInfo_[subPicIndex].subpicCbQpIndexOffset, subpicSyntaxInfo_[subPicIndex].subpicCrQpIndexOffset);
            }
            llDecoder->LLDecode(&bitstream_, &seqPicHeaderInfo_, &subpicSyntaxInfo_[subPicIndex]);

            hfDecoder->Set(subpicSyntaxInfo_[subPicIndex].subpicWidth >> 1, subpicSyntaxInfo_[subPicIndex].subpicHeight >> 1,
                subPicDec_->subBands_, qp_, subpicSyntaxInfo_[subPicIndex].subpicCbQpIndexOffset, subpicSyntaxInfo_[subPicIndex].subpicCrQpIndexOffset,
                subpicSyntaxInfo_[subPicIndex].subpicHlQpIndexOffset, subpicSyntaxInfo_[subPicIndex].subpicLhQpIndexOffset, subpicSyntaxInfo_[subPicIndex].subpicHhQpIndexOffset);
            hfDecoder->HFDecode(&bitstream_, &seqPicHeaderInfo_, &subpicSyntaxInfo_[subPicIndex]);


            Status decStatus = RecImage(subPicDec_->subBands_, subPicDec_->subPicInfo_[subPicIndex],
                subPicInfoDec_, subPicLLInfoDec_, subPicLLInfoRef_, subPicDec_->dwtRowBuffer_, subPicDec_->dwtTransTmpBuffer_,
                picWHRaw_, picWHRawLL_, decFile_, decLLFile_, bitDepth_, needRef);
            if (decStatus != Status::SUCCESS) {
                LOGE("%s\n", "decode reconstructing image failed.");
            }

            // TODO: stream update
            bitstream_.streamBuffer += subpicSyntaxInfo_[subPicIndex].subpicLength;
            curBitstreamPos_ += subpicSyntaxInfo_[subPicIndex].subpicLength;
        }
        bitstream_.streamBuffer += bitstream_.frame_bitoffset >> 3;
        bitstream_.frame_bitoffset = 0;
        if (!decFile_.empty()) {
            WriteRecPic(decPicBuffer_, picWHRaw_, decFileHandle_);
        }
        if (!decLLFile_.empty()) {
            WriteRecPic(decPicLLBuffer_, picWHRawLL_, decLLFileHandle_);
        }
    }

}
