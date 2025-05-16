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
#include <algorithm>
#include <string>
#include "Encoder.h"
#include "Log.h"
#include "RecImage.h"
#include "Utils.h"
#include "Wavelet.h"
#include "biariencode.h"
#include "cabac.h"
#include "context_ini.h"

namespace HFM {
    Encoder::Encoder(std::string recFile, std::string recLLFile, uint32_t frameCount, uint32_t bitDepth, uint32_t intraPeriod)
        : recFile_(std::move(recFile)), recLLFile_(std::move(recLLFile)), frameCount_(frameCount), bitDepth_(bitDepth), intraPeriod_(intraPeriod) {
        if (!recFile_.empty()) {
            recFileHandle_.open(recFile_, std::ofstream::binary | std::ofstream::trunc);
            if (recFileHandle_.fail()) {
                LOGE("failed to open: %s\n", recFile_.c_str());
            } else {
                LOGI("open: %s\n", recFile_.c_str());
            }
        }

        if (!recLLFile_.empty()) {
            recLLFileHandle_.open(recLLFile_, std::ofstream::binary | std::ofstream::trunc);
            if (recLLFileHandle_.fail()) {
                LOGE("failed to open: %s\n", recLLFile_.c_str());
            } else {
                LOGI("open: %s\n", recLLFile_.c_str());
            }
        }
    }

    Encoder::~Encoder() {
        if (recFileHandle_.is_open()) {
            recFileHandle_.close();
        }
        if (recLLFileHandle_.is_open()) {
            recLLFileHandle_.close();
        }
    }

    void Encoder::SetInput(std::shared_ptr<SubPicEnc> subPic, int32_t qp, int32_t cbQpOffset, int32_t crQpOffset, int32_t hlQpOffset, 
        int32_t lhQpOffset, int32_t hhQpOffset, bool qpDeltaEnable, bool hfTransformSkip) {
        subPic_ = std::move(subPic);
        cbQpOffset_ = cbQpOffset;
        crQpOffset_ = crQpOffset;
        hlQpOffset_ = hlQpOffset;
        lhQpOffset_ = lhQpOffset;
        hhQpOffset_ = hhQpOffset;
        qpDeltaEnable_ = qpDeltaEnable;
        hfTransformSkip_ = hfTransformSkip;
        Clip<int32_t, uint8_t>(qp, qp_, 0, 39);
        if (!recFile_.empty() && !recPicBuffer_) {
            subPicInfoRec_ = subPic_->subPicInfo_;
            uint32_t picPixels[N_COLOR_COMP] {};
            for (auto color : COLORS) {
                auto comp = color == Y ? LUMA : CHROMA;
                recPicSize_[comp] = subPic_->GetPicSizeRaw(color);
                // rec image size is W x H, stored in W x H buffer
                recPicSize_[comp].strideW = recPicSize_[comp].w;
                recPicSize_[comp].strideH = recPicSize_[comp].h;
                picPixels[color] += recPicSize_[comp].strideW * recPicSize_[comp].strideH;
                for (auto & info : subPicInfoRec_) {
                    info[color].strideW = recPicSize_[comp].w;
                    info[color].strideH = recPicSize_[comp].h;
                }
                picWHRaw_.emplace_back(recPicSize_[comp].w, recPicSize_[comp].h);
            }
            if (!recPicBuffer_) {
                recPicBuffer_ = std::make_shared<BufferStorage>(picPixels[Y] + picPixels[U] + picPixels[V], 0);
                PelStorage* picHeaderPtr[N_COLOR_COMP] {};
                picHeaderPtr[Y] = recPicBuffer_->data();
                picHeaderPtr[U] = picHeaderPtr[Y] + picPixels[Y];
                picHeaderPtr[V] = picHeaderPtr[U] + picPixels[U];
                for (auto & info : subPicInfoRec_) {
                    for (auto color: COLORS) {
                        info[color].picHeaderPtr = picHeaderPtr[color];
                    }
                }
            }
        }
        if (!recPicLLBuffer_) {
            subPicLLInfoRec_ = subPic_->subPicInfo_;
            uint32_t picLLPixels[N_COLOR_COMP] {};
            for (auto color : COLORS) {
                auto picSize = subPic_->GetPicSizeRaw(color);
                // low pass sub-band element count is (inputLen + 1) >> 1
                uint32_t widthLL = (picSize.w + 1) >> 1;
                uint32_t heightLL = (picSize.h + 1) >> 1;
                picLLPixels[color] += widthLL * heightLL;
                for (auto & info : subPicLLInfoRec_) {
                    info[color].w >>= 1;
                    info[color].h >>= 1;
                    info[color].x >>= 1;
                    info[color].y >>= 1;
                    info[color].strideW = widthLL;
                    info[color].strideH = heightLL;
                }
                picWHRawLL_.emplace_back(widthLL, heightLL);
            }
            recPicLLBuffer_ = std::make_shared<BufferStorage>(picLLPixels[Y] + picLLPixels[U] + picLLPixels[V], 0);
            PelStorage* picLLHeaderPtr[N_COLOR_COMP] {};
            picLLHeaderPtr[Y] = recPicLLBuffer_->data();
            picLLHeaderPtr[U] = picLLHeaderPtr[Y] + picLLPixels[Y];
            picLLHeaderPtr[V] = picLLHeaderPtr[U] + picLLPixels[U];
            for (auto & info : subPicLLInfoRec_) {
                for (auto color: COLORS) {
                    info[color].picHeaderPtr = picLLHeaderPtr[color];
                }
            }
        }
        {
            if (!refPicLLBuffer_) {
                subPicLLInfoRef_ = subPic_->subPicInfo_;
                uint32_t picLLPixels[N_COLOR_COMP] {};
                for (auto color : COLORS) {
                    auto picSize = subPic_->GetPicSizeRaw(color);
                    // low pass sub-band element count is (inputLen + 1) >> 1
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

    void Encoder::StreamCabacCtxIni() {
        // vlc stream
        BitstreamInit(&bitstreamVlcLl_, 24);
        BitstreamInit(&bitstreamVlcHf_, 24);
        // cabac ee and stream
        eeCabacLl_ = {0};
        eeCabacHf_ = {0};
#if CABAC
        BitstreamInit(&bitstreamCabacLl_, 24);
        arienco_start_encoding(&eeCabacLl_, bitstreamCabacLl_.streamBuffer, &(bitstreamCabacLl_.byte_pos));
        arienco_reset_EC(&eeCabacLl_);
#endif
#if CABAC_HF
        BitstreamInit(&bitstreamCabacHf_, 24);
        arienco_start_encoding(&eeCabacHf_, bitstreamCabacHf_.streamBuffer, &(bitstreamCabacHf_.byte_pos));
        arienco_reset_EC(&eeCabacHf_);
#endif
        // ctx ini
#if CABAC
        InitContextsLl(qp_, &texCtx_, &motCtx_);
#endif
#if CABAC_HF
        InitContextsHf(qp_, &highBandCtx_);
#endif
    }

    void Encoder::SubpicEncodingDone(uint32_t subpicIndex) {
        //stuffing vlc
        WriteStuffingBits(&bitstreamVlcLl_);
        WriteStuffingBits(&bitstreamVlcHf_);
        // terminate the arithmetic code
#if CABAC
        biari_encode_symbol_final(&eeCabacLl_, 1);
        arienco_done_encoding(&eeCabacLl_);
        int iCabacLfEcodestrm_len = *eeCabacLl_.Ecodestrm_len;
#else
        int iCabacLfEcodestrm_len = 0;
#endif
#if CABAC_HF
        biari_encode_symbol_final(&eeCabacHf_, 1);
        arienco_done_encoding(&eeCabacHf_);
        int iCabacHfEcodestrm_len = *eeCabacHf_.Ecodestrm_len;
#else
        int iCabacHfEcodestrm_len = 0;
#endif
        //write qp info/subpic size
        
        //qp to be added
        int subPicBytes = bitstreamVlcLl_.byte_pos + bitstreamVlcHf_.byte_pos + iCabacLfEcodestrm_len + iCabacHfEcodestrm_len;
        byte *picSubpicInfoPointer = bitstream_.streamBuffer + seqHeaderBytes_ + 7 + 17 * subpicIndex; //7 is pic size, slice type...bytes

        int qpCombine = (qp_ << 26) + ((hlQpOffset_ + 12) << 21) + ((lhQpOffset_ + 12) << 16) \
            + ((hhQpOffset_ + 12) << 11) + ((cbQpOffset_ + 12) << 6) + ((crQpOffset_ + 12) << 1);
        picSubpicInfoPointer[0] = qpCombine >> 24;
        picSubpicInfoPointer[1] = (qpCombine >> 16) & 0xff;
        picSubpicInfoPointer[2] = (qpCombine >> 8) & 0xff;
        picSubpicInfoPointer[3] = qpCombine & 0xff;

        picSubpicInfoPointer[5] = subPicBytes >> 16;
        picSubpicInfoPointer[6] = (subPicBytes >> 8) & 0xff;
        picSubpicInfoPointer[7] = subPicBytes & 0xff;
        picSubpicInfoPointer[8] = iCabacLfEcodestrm_len >> 16;
        picSubpicInfoPointer[9] = (iCabacLfEcodestrm_len >> 8) & 0xff;
        picSubpicInfoPointer[10] = iCabacLfEcodestrm_len & 0xff;
        picSubpicInfoPointer[11] = bitstreamVlcLl_.byte_pos >> 16;
        picSubpicInfoPointer[12] = (bitstreamVlcLl_.byte_pos >> 8) & 0xff;
        picSubpicInfoPointer[13] = bitstreamVlcLl_.byte_pos & 0xff;
        picSubpicInfoPointer[14] = iCabacHfEcodestrm_len >> 16;
        picSubpicInfoPointer[15] = (iCabacHfEcodestrm_len >> 8) & 0xff;
        picSubpicInfoPointer[16] = iCabacHfEcodestrm_len & 0xff;
#if CABAC
        memcpy(bitstream_.streamBuffer + bitstream_.byte_pos , eeCabacLl_.Ecodestrm, iCabacLfEcodestrm_len);
#endif
        memcpy(bitstream_.streamBuffer + bitstream_.byte_pos + iCabacLfEcodestrm_len, bitstreamVlcLl_.streamBuffer, bitstreamVlcLl_.byte_pos);
#if CABAC_HF
        memcpy(bitstream_.streamBuffer + bitstream_.byte_pos + *eeCabacLl_.Ecodestrm_len + bitstreamVlcLl_.byte_pos, eeCabacHf_.Ecodestrm, *eeCabacHf_.Ecodestrm_len);
#endif
        memcpy(bitstream_.streamBuffer + bitstream_.byte_pos + iCabacLfEcodestrm_len + bitstreamVlcLl_.byte_pos + iCabacHfEcodestrm_len, bitstreamVlcHf_.streamBuffer, bitstreamVlcHf_.byte_pos);
#if CABAC
        BitstreamRelease(&bitstreamCabacLl_);
#endif
#if CABAC_HF
        BitstreamRelease(&bitstreamCabacHf_);
#endif
        BitstreamRelease(&bitstreamVlcLl_);
        BitstreamRelease(&bitstreamVlcHf_);

        bitstream_.byte_pos += subPicBytes;
    }

    void Encoder::PicEncodingDone() {
        int frameByte = bitstream_.byte_pos - seqHeaderBytes_;
        LOGI("BYTES %d\n", bitstream_.byte_pos);
        byte *picSizePointer = bitstream_.streamBuffer + seqHeaderBytes_;
        picSizePointer[0] = frameByte >> 24;
        picSizePointer[1] = (frameByte >> 16) & 0xff;
        picSizePointer[2] = (frameByte >> 8) & 0xff;
        picSizePointer[3] = frameByte & 0xff;
    }


    void Encoder::DetermineMBdeltaQP(uint32_t bandWidth, uint32_t bandHeight) {
        uint32_t maxWidthMb = (bandWidth + MB_SIZE - 1) / MB_SIZE;
        uint32_t maxHeightMb = (bandHeight + MB_SIZE - 1) / MB_SIZE;
        mbDeltaQP_.resize(maxHeightMb);
        uint32_t stride = bandWidth;
        for (uint32_t mbY = 0; mbY < maxHeightMb; mbY++) {
            mbDeltaQP_[mbY].resize(maxWidthMb);
            for (uint32_t mbX = 0; mbX < maxWidthMb; mbX++) {
                uint32_t pos = (mbY * MB_SIZE) * bandWidth + mbX * MB_SIZE;
                uint32_t mean8x8 = 0;
                uint32_t pixel = 0;
                uint32_t picIndex = pos;
                uint64_t sum[4] = {0};
                uint64_t sum2[4] = {0};
                uint64_t sum8x8 = 0;
                uint64_t sumSqr8x8 = 0;
                int64_t var[4] = {0};
                int64_t varMax;
                int64_t varMin = 0;
                int64_t var8x8 = 0;
                int maxWidthMb = (stride + MB_SIZE - 1) / MB_SIZE;
                int mbXpos = pos % stride;
                bool isSubpicBoud = (mbXpos == 0) || (mbXpos / MB_SIZE == maxWidthMb - 1);
                for (uint8_t yPos = 0; yPos < 8; ++yPos) {
                    for (uint8_t xPos = 0; xPos < 8; ++xPos) {
                        int blkXidx = xPos / 4;
                        int blkYidx = yPos / 4;
                        pixel = subPic_->subBands_[LL][Y]->at(picIndex + xPos);
                        sum[blkYidx * 2 + blkXidx] += (uint64_t)pixel;
                        sum2[blkYidx * 2 + blkXidx] += (uint64_t)pixel*pixel;
                        sum8x8 += (uint64_t)pixel;
                        sumSqr8x8 += (uint64_t)pixel*pixel;
                    }
                    picIndex += stride;
                }
                var8x8 = (int64_t)(sumSqr8x8 - ((sum8x8*sum8x8 + 32) >> 6)); //32-bit
                var8x8 = (var8x8 + (1 << 8)) >> 9;
                mean8x8 = (sum8x8 + 32) >> 6;
                for (uint8_t yPos = 0; yPos < 2; ++yPos) {
                    for (uint8_t xPos = 0; xPos < 2; ++xPos) {
                        var[yPos * 2 + xPos] = (int64_t)(sum2[yPos * 2 + xPos] - ((sum[yPos * 2 + xPos] * sum[yPos * 2 + xPos] + 8) >> 4));
                        if ((xPos == 0) && (yPos == 0)) {
                            varMax = var[yPos * 2 + xPos];
                            varMin = var[yPos * 2 + xPos];
                        } else {
                            if (var[yPos * 2 + xPos] > varMax)
                                varMax = var[yPos * 2 + xPos];
                            if (var[yPos * 2 + xPos] < varMin)
                                varMin = var[yPos * 2 + xPos];
                        }
                    }
                }

                varMax = (varMax + (1 << 8)) >> 9;
                varMin = (varMin + (1 << 8)) >> 9;
                int mbQpDeltaA = 0;
                {
                    //printf("\t %d,%d,%d,", var8x8 , varMin << 2, varMax << 2);
                    int varA = var8x8;
                    if ((varMax - varMin) > (50000 >> 2))
                        varA = varMin << 2;
                    int varDiffAbs = (varA - 4096);
                    int signFlag = varA < 4096 ? -1 : 1;
                    int varStrengthTemp = 2;
                    int varShiftTemp = 2;

                    if (signFlag > 0) {
                        varStrengthTemp = 4;
                        varShiftTemp = 2;
                    }

                    varDiffAbs = abs(varDiffAbs);
                    int varDeltaTemp = log2((varDiffAbs / 64) + 1);
                    mbQpDeltaA = (varDeltaTemp*varStrengthTemp) >> varShiftTemp;
                    Clip(mbQpDeltaA * signFlag, mbQpDeltaA, -16, 15);

                }

                int mbQpDeltaB = 0;
                {
                    const int deltaQpMap[2][5] = {
                        {0,0,0,0,0},
                        {-8,-7,-5,0,0},
                    };
                    int qpOffset = 0;
                    if (qp_ > 12)
                        qpOffset = (qp_ - 12) / 2;
                    int lumaIndx = (mean8x8 < ((200 * 4) + (1 << 11))) ? 0 : 1;
                    int varIndx = 4;
                    if (var8x8 < (10 * 8)) {
                        varIndx = 0;
                    } else if (var8x8 < (25 * 8)) {
                        varIndx = 1;
                    } else if (var8x8 < (50 * 8)) {
                        varIndx = 2;
                    } else if (var8x8 < (100 * 8)) {
                        varIndx = 3;
                        qpOffset = 0;
                    } else {
                        varIndx = 3;
                        qpOffset = 0;
                    }
                    mbQpDeltaB = deltaQpMap[lumaIndx][varIndx] + qpOffset;
                }

                int mbQpDeltaC = 0;
                if (isSubpicBoud && (var8x8 < 10000)) {
                    int curMbQP;
                    Clip(qp_ + mbQpDeltaA + mbQpDeltaB, curMbQP, 0, 39);
                    //curMbQP = qp_[Y] + mbQpDeltaA + mbQpDeltaB + mbQpDeltaC;
                    if (curMbQP > 8)
                        mbQpDeltaC = 8 - qp_ - mbQpDeltaA - mbQpDeltaB;
                }
                mbDeltaQP_[mbY][mbX]= mbQpDeltaA + mbQpDeltaB + mbQpDeltaC;
            }
        }
    }


    void Encoder::Encode(uint32_t currFrame) {
        auto frameType = static_cast<FrameType>(currFrame % intraPeriod_);
        bool needRef = frameType == FRAME_I && frameCount_ > 1 && intraPeriod_ > 1;
        auto llEncoder = std::make_shared<LLEncoder>();
        auto hfEncoder = std::make_shared<HFEncoder>();
        for (auto & subPicInfo : subPic_->subPicInfo_) {
            uint32_t subPicId = subPicInfo[Y].id;
            StreamCabacCtxIni();
            subPic_->DWT(subPicInfo);
            DetermineMBdeltaQP(subPicInfo[Y].w >> 1, subPicInfo[Y].h >> 1);
            if (frameType == FRAME_P) {
                subPic_->SetLLReference(subPicLLInfoRef_[subPicId]);
                llEncoder->Set(subPicInfo[Y].w >> 1, subPicInfo[Y].h >> 1,
                    subPic_->subBands_[LL], subPic_->subBandsRec_[LL], subPic_->subBandsRef_[LL], 
                    qp_, cbQpOffset_, crQpOffset_);
            } else {
                llEncoder->Set(subPicInfo[Y].w >> 1, subPicInfo[Y].h >> 1,
                    subPic_->subBands_[LL], subPic_->subBandsRec_[LL], qp_, cbQpOffset_, crQpOffset_);
            }
            llEncoder->LLEncode(&bitstreamVlcLl_, &eeCabacLl_, &texCtx_, &motCtx_, frameType, qpDeltaEnable_, mbDeltaQP_);

            hfEncoder->Set(subPicInfo[Y].w >> 1, subPicInfo[Y].h >> 1,
                subPic_->subBands_, subPic_->subBandsRec_,
                qp_, cbQpOffset_, crQpOffset_, hlQpOffset_, lhQpOffset_, hhQpOffset_);
            hfEncoder->HFEncode(&bitstreamVlcHf_, &eeCabacHf_, &highBandCtx_, qpDeltaEnable_, mbDeltaQP_, hfTransformSkip_);

            Status recStatus = RecImage(subPic_->subBandsRec_, subPicInfo, subPicInfoRec_, subPicLLInfoRec_, subPicLLInfoRef_, 
                                        subPic_->dwtRowBuffer_, subPic_->dwtTransTmpBuffer_,
                                        picWHRaw_, picWHRawLL_, recFile_, recLLFile_, bitDepth_, needRef);
            if (recStatus != Status::SUCCESS) {
                LOGE("%s\n", "Reconstruct image failed.");
            }
            SubpicEncodingDone(subPicInfo[Y].id);
        }
        PicEncodingDone();

        if (!recFile_.empty()) {
            WriteRecPic(recPicBuffer_, picWHRaw_, recFileHandle_);
            auto psnr = CalcPSNR(subPic_->storageBuffer_->data(), recPicBuffer_->data(),
                                 subPic_->picSize_, recPicSize_, bitDepth_, PSNR_1023);
            LOGI("PSNR Y = %5.2f, U = %5.2f, V = %5.2f\n", psnr[Y], psnr[U], psnr[V]);
        }
        if (!recLLFile_.empty()) {
            WriteRecPic(recPicLLBuffer_, picWHRawLL_, recLLFileHandle_);
        }
    }
}
