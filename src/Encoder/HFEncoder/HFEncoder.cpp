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
#include "HFEncoder.h"

namespace HFM {
    HFEncoder::HFEncoder() {
        mbOri_.resize(MB_SIZE*MB_SIZE);
        mbHad_.resize(MB_SIZE*MB_SIZE);
        mbCoeff_.resize(MB_SIZE*MB_SIZE);
        mbRec_.resize(MB_SIZE*MB_SIZE);
        encoderQuant_ = std::make_shared<EncoderQuant>();
        hfIQuant_ = std::make_shared<IQuant>();
        hfTransITrans_ = std::make_shared<HFTransITrans>();
        hfEncoderEntropy_ = std::make_shared<HFEncoderEntropy>();
    }

    HFEncoder::~HFEncoder() {
    }


    void HFEncoder::hfMBOriReorder() {
        for (uint8_t i = 0; i < (MB_SIZE*MB_SIZE >> componentShiftX_); i++) {
            mbOri_[i] = (*subPic_)[hfBandIdx_][colorComponent_]->at(pixelIndex_ + lineWidth_ * HF_REMAP_LINE[i] + HF_REMAP_POS_16[i / 32 * 8 + i % 8]);
        }
    }

    void HFEncoder::hfMBRecReorder(uint8_t bestTransType) {
        FrameBuffer* ptr;
        //ptr = (bestTransType == HF_NOHAD) ? &mbNohadRec_ : &mbHadRec_;
        ptr = &mbRec_;
        for (uint8_t i = 0; i < (MB_SIZE*MB_SIZE >> componentShiftX_); i++) {
            (*subPicRec_)[hfBandIdx_][colorComponent_]->at(pixelIndex_ + lineWidth_ * HF_REMAP_LINE[i] + HF_REMAP_POS_16[i / 32 * 8 + i % 8]) = (*ptr)[i];
        }
    }


    void HFEncoder::Set(uint32_t hfBandWidth, uint32_t hfBandHeight, SubBandMap& subPic, SubBandMap& subPicRec,
        uint8_t qp, int32_t cbQpOffset, int32_t crQpOffset, int32_t hlQpOffset, int32_t lhQpOffset, int32_t hhQpOffset) {
        hfBandWidth_ = hfBandWidth;
        hfBandHeight_ = hfBandHeight;
        subPic_ = &subPic;
        subPicRec_ = &subPicRec;
        Clip<int8_t, uint8_t>(qp + hlQpOffset, qp_[HL][Y], 0, 39);
        Clip<int8_t, uint8_t>(qp + lhQpOffset, qp_[LH][Y], 0, 39);
        Clip<int8_t, uint8_t>(qp + hhQpOffset, qp_[HH][Y], 0, 39);
        for (hfBandIdx_ = HL; hfBandIdx_ <= HH; hfBandIdx_++) {
            Clip<int8_t, uint8_t>(qp_[hfBandIdx_][Y] + cbQpOffset, qp_[hfBandIdx_][U], 0, 39);
            Clip<int8_t, uint8_t>(qp_[hfBandIdx_][Y] + crQpOffset, qp_[hfBandIdx_][V], 0, 39);
        }
    }

    uint32_t HFEncoder::SimpleRd(uint8_t qp, FrameBuffer &mbPix) {
        uint16_t quantLeft = 0;
        uint16_t coeff = 0;
        uint16_t sig;
        uint16_t rightShift = 0;
        uint16_t leftShift = 0;
        uint16_t qpIndex = qp;
        uint16_t dis = 0;
        uint16_t rate = 0;
        pel tempPix;

        qpIndex += 4;
        if (qpIndex < 16) {
            leftShift = 2 - (qpIndex >> 3);
        }

        if (qpIndex > 23) {
            rightShift = (qpIndex >> 3) - 2;
        }
        qpIndex &= 7;

        int qScale = SIMP_QUANT_SCALE[qpIndex];
        int qShift = SIMP_QUANT_SHIFT[qpIndex];
        int deadzone = SIMP_DEADZONE[qpIndex];
        int round = SIMP_ROUND[qpIndex];
        int quantsize = 1 << qShift;
        int quantsizeMin1 = quantsize - 1;

        for (int i = 0; i < (64); i++) {
            coeff = abs(mbPix[i]);
            coeff <<= leftShift;
            coeff >>= rightShift;

            coeff *= qScale;
            quantLeft = (coeff & quantsizeMin1);
            tempPix = coeff >> qShift;
            if (coeff > deadzone && quantLeft > round) {
                quantLeft = (quantsize - quantLeft);
                tempPix++;
            }
            Clip(tempPix, tempPix, 0, 255);
            quantLeft <<= rightShift;
            quantLeft >>= leftShift;
            dis += quantLeft;
            rate += VLCTable0(tempPix);
        }
        uint16_t lambda = SIMP_LAMBDA[qp];
        return (rate * lambda + dis);
    }

    void HFEncoder::ChromaEnhanceQuant(FrameBuffer& mbPix) {
        uint8_t mbNumber = 2;
        for (int mbIdx = 0; mbIdx < mbNumber; mbIdx++) {
            uint32_t sum=0;
            for (int i = 0; i < 16; i++) {
                sum += abs(mbPix[i+ mbIdx*16]);
            }
            if (sum == 1) {
                memset(&mbPix[mbIdx * 16], 0, sizeof(Pel)*16);
            }
        }
    };

    void HFEncoder::HFEncode(Bitstream* bitstreamVlcHf_, EncodingEnvironment* eeCabacHf_, HighBandInfoContexts* highBandCtx_, bool qpDeltaEnable, std::vector<std::vector<int16_t>>& mbDeltaQP, bool hfTransformSkip) {
        hfEncoderEntropy_->Set(bitstreamVlcHf_, eeCabacHf_, highBandCtx_);

        uint32_t maxWidthMb = (hfBandWidth_ + MB_SIZE - 1) / MB_SIZE;
        uint32_t maxHeightMb = (hfBandHeight_ + MB_SIZE - 1) / MB_SIZE;
        uint8_t refQP[N_SUB_BANDS][N_COLOR_COMP] = {0};
        FrameBuffer* ptr;
        for (uint32_t mbY = 0; mbY < maxHeightMb; mbY++) {
            for (uint32_t mbX = 0; mbX < maxWidthMb; mbX++) {
                if (qpDeltaEnable) {
                    int curMbQP; //to be replaced by AQ
                    int qpMBdelta;
                    uint32_t pos = (mbY * MB_SIZE) * hfBandWidth_ + mbX * MB_SIZE;
                    Clip(qp_[HL][Y] + mbDeltaQP[mbY][mbX], curMbQP, 0, 39);
                    hfEncoderEntropy_->HFEntropyDeltaQp(mbX == 0, qp_[HL][Y], curMbQP, qpMBdelta);

                    for (int bandidx = HL; bandidx <= HH; bandidx++) {
                        for (int comIdx = Y; comIdx <= V; comIdx++) {
                            if (mbX == 0) {
                                Clip(qp_[bandidx][comIdx] + qpMBdelta, mbQp_[bandidx][comIdx], 0, 39);
                                refQP[bandidx][comIdx] = mbQp_[bandidx][comIdx];
                            } else {
                                Clip(refQP[bandidx][comIdx] + qpMBdelta, mbQp_[bandidx][comIdx], 0, 39);
                                refQP[bandidx][comIdx] = mbQp_[bandidx][comIdx];
                            }
                        }
                    }

                } else {
                    for (int bandidx = HL; bandidx <= HH; bandidx++) {
                        for (int comIdx = Y; comIdx <= V; comIdx++) {
                            mbQp_[bandidx][comIdx] = qp_[bandidx][comIdx];
                        }
                    }
                }


                for (hfBandIdx_ = HL; hfBandIdx_ <= HH; hfBandIdx_++) {
                    for (colorComponent_ = Y; colorComponent_ <= V; colorComponent_++) {
                        componentShiftX_ = (colorComponent_ == Y) ? 0 : 1;
                        lineWidth_ = hfBandWidth_ >> componentShiftX_;
                        pixelIndex_ = mbY * MB_SIZE  * lineWidth_ + mbX * (MB_SIZE >> componentShiftX_);
                        uint8_t curMbQP = mbQp_[hfBandIdx_][colorComponent_];
                        hfMBOriReorder();
                        hfTransITrans_->Set(colorComponent_, 0);
                        hfTransITrans_->ComHFTransITrans(mbOri_, mbHad_);
                        if (hfTransformSkip) {
                            bestTransType_ = ((colorComponent_ == Y && SimpleRd(curMbQP, mbOri_) < SimpleRd(curMbQP, mbHad_)) ? HF_NOHAD : HF_HAD);
                        } else {
                            bestTransType_ = HF_HAD;
                        }
                        ptr = (bestTransType_ == HF_NOHAD) ? &mbOri_ : &mbHad_;

                        encoderQuant_->Set((MB_SIZE >> componentShiftX_), MB_SIZE, curMbQP, HF_DEADZONE, HF_TH1, HF_RDOQ_OFFSET, HF_QUANT_DYNAMIC_BIT);
                        encoderQuant_->Quant(*ptr, mbCoeff_, 0);

                        if (colorComponent_ != Y) {
                            ChromaEnhanceQuant(mbCoeff_);
                        }

                        hfEncoderEntropy_->HFEntropyCoeffGroupSet(mbX == 0, hfBandIdx_, colorComponent_, bestTransType_);
                        hfEncoderEntropy_->HFEntropyCoeffGroup(mbCoeff_, &hfEncoderEntropy_->bgParams_, hfTransformSkip);

                        hfIQuant_->Set((MB_SIZE >> componentShiftX_), MB_SIZE, curMbQP, HF_IQUANT_DYNAMIC_BIT);
                        hfIQuant_->ComIQuant(mbCoeff_, mbRec_, 0);

                        if (bestTransType_ == HF_HAD) {
                            hfTransITrans_->Set(colorComponent_, 1);
                            hfTransITrans_->ComHFTransITrans(mbRec_, mbRec_);
                        }

                        hfMBRecReorder(bestTransType_);
                    }
                }
            }
        }

    }
}