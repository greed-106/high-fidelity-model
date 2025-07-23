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


    void HFEncoder::Set(PixelFormat pixelFormat, uint32_t hfBandWidth, uint32_t hfBandHeight, SubBandMap& subPic, SubBandMap& subPicRec,
        uint8_t qp, QPGroup qpGroup) {
        pixelFormat_ = pixelFormat;
        hfBandWidth_ = hfBandWidth;
        hfBandHeight_ = hfBandHeight;
        subPic_ = &subPic;
        subPicRec_ = &subPicRec;
        Clip<int8_t, uint8_t>(qp + qpGroup.hlQpOffset, qp_[HL][Y], 0, 39);
        Clip<int8_t, uint8_t>(qp + qpGroup.lhQpOffset, qp_[LH][Y], 0, 39);
        Clip<int8_t, uint8_t>(qp + qpGroup.hhQpOffset, qp_[HH][Y], 0, 39);
        for (hfBandIdx_ = HL; hfBandIdx_ <= HH; hfBandIdx_++) {
            Clip<int8_t, uint8_t>(qp_[hfBandIdx_][Y] + qpGroup.cbQpOffset, qp_[hfBandIdx_][U], 0, 39);
            Clip<int8_t, uint8_t>(qp_[hfBandIdx_][Y] + qpGroup.crQpOffset, qp_[hfBandIdx_][V], 0, 39);
        }
    }

    void HFEncoder::ChromaEnhanceQuant(FrameBuffer& mbPix) {
        uint8_t mbNumber = (pixelFormat_ == PixelFormat::YUV422P10LE ? 2 : 4);
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

    uint32_t HFEncoder::Distortion() {
        uint32_t sse = 0;
        int32_t tmp = 0;
        for (uint8_t pos = 0; pos < MB_SIZE * MB_SIZE; ++pos) {
                tmp = mbOri_[pos] - mbRec_[pos];
                sse += tmp * tmp;
            }
        return sse;
    }

    void HFEncoder::HFEncode(Bitstream* bitstreamVlcHf_, EncodingEnvironment* eeCabacHf_, HighBandInfoContexts* highBandCtx_, bool qpDeltaEnable, std::vector<std::vector<int16_t>>& mbDeltaQP, bool hfTransformSkip) {
        hfEncoderEntropy_->Set(bitstreamVlcHf_, eeCabacHf_, highBandCtx_);

        uint32_t maxWidthMb = (hfBandWidth_ + MB_SIZE - 1) / MB_SIZE;
        uint32_t maxHeightMb = (hfBandHeight_ + MB_SIZE - 1) / MB_SIZE;
        uint8_t refQP[N_SUB_BANDS][N_YUV_COMP] = {0};
        FrameBuffer* ptr;
        for (uint32_t mbY = 0; mbY < maxHeightMb; mbY++) {
            for (uint32_t mbX = 0; mbX < maxWidthMb; mbX++) {
                if (qpDeltaEnable) {
                    int curMbQP; //to be replaced by AQ
                    int qpMBdelta;
                    uint32_t pos = (mbY * MB_SIZE) * hfBandWidth_ + mbX * MB_SIZE;
                    Clip<int, int>(qp_[HL][Y] + mbDeltaQP[mbY][mbX], curMbQP, 0, 39);
                    hfEncoderEntropy_->HFEntropyDeltaQp(mbX == 0, qp_[HL][Y], curMbQP, qpMBdelta);

                    for (int bandidx = HL; bandidx <= HH; bandidx++) {
                        for (int comIdx = Y; comIdx <= V; comIdx++) {
                            if (mbX == 0) {
                                Clip<int8_t, uint8_t>(qp_[bandidx][comIdx] + qpMBdelta, mbQp_[bandidx][comIdx], 0, 39);
                                refQP[bandidx][comIdx] = mbQp_[bandidx][comIdx];
                            } else {
                                Clip<int8_t, uint8_t>(refQP[bandidx][comIdx] + qpMBdelta, mbQp_[bandidx][comIdx], 0, 39);
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
                        componentShiftX_ = (colorComponent_ != Y&& pixelFormat_ == PixelFormat::YUV422P10LE) ? 1 : 0;
                        lineWidth_ = hfBandWidth_ >> componentShiftX_;
                        pixelIndex_ = mbY * MB_SIZE  * lineWidth_ + mbX * (MB_SIZE >> componentShiftX_);
                        uint8_t curMbQP = mbQp_[hfBandIdx_][colorComponent_];
                        hfMBOriReorder();
                        hfTransITrans_->Set(componentShiftX_, 0);
                        hfTransITrans_->ComHFTransITrans(mbOri_, mbHad_);
                        bestTransType_ = HF_HAD;
                        if (colorComponent_ == Y && hfTransformSkip) {
                            float cost[2] = {0,0};
                            uint32_t dis=0, rate=0;
                            for (uint8_t transType = HF_HAD; transType <= HF_NOHAD; transType++) {
                                ptr = (transType == HF_NOHAD) ? &mbOri_ : &mbHad_;
                                encoderQuant_->Set((MB_SIZE >> componentShiftX_), MB_SIZE, curMbQP, HF_DEADZONE, HF_TH1, HF_RDOQ_OFFSET, HF_QUANT_DYNAMIC_BIT);
                                encoderQuant_->Quant(*ptr, mbCoeff_, 0);
                                hfEncoderEntropy_->GetCabcaState();
                                int tmpLeftMaxCoeff = hfEncoderEntropy_->bgParams_.leftCoefMax[3* (hfBandIdx_-1)];
                                hfEncoderEntropy_->HFEntropyCoeffGroupSet(mbX == 0, hfBandIdx_, colorComponent_, componentShiftX_, bestTransType_);
                                hfEncoderEntropy_->HFEntropyCoeffGroup(mbCoeff_, &hfEncoderEntropy_->bgParams_, hfTransformSkip);

                                hfIQuant_->Set((MB_SIZE >> componentShiftX_), MB_SIZE, curMbQP, HF_IQUANT_DYNAMIC_BIT);
                                hfIQuant_->ComIQuant(mbCoeff_, mbRec_, 0);

                                if (transType == HF_HAD) {
                                    hfTransITrans_->Set(colorComponent_, 1);
                                    hfTransITrans_->ComHFTransITrans(mbRec_, mbRec_);
                                }
                                dis = Distortion();
                                rate = hfEncoderEntropy_->bits_;
                                hfEncoderEntropy_->ResetCabcaState();
                                hfEncoderEntropy_->bgParams_.leftCoefMax[3 * (hfBandIdx_ - 1)]=tmpLeftMaxCoeff;
                                cost[transType]=RdCostCal(rate, dis, curMbQP);
                            }
                            bestTransType_ = (cost[HF_HAD] <= cost[HF_NOHAD] ? HF_HAD : HF_NOHAD);
                        }
                        ptr = (bestTransType_ == HF_NOHAD) ? &mbOri_ : &mbHad_;

                        encoderQuant_->Set((MB_SIZE >> componentShiftX_), MB_SIZE, curMbQP, HF_DEADZONE, HF_TH1, HF_RDOQ_OFFSET, HF_QUANT_DYNAMIC_BIT);
                        encoderQuant_->Quant(*ptr, mbCoeff_, 0);

                        if (colorComponent_ != Y) {
                            ChromaEnhanceQuant(mbCoeff_);
                        }

                        hfEncoderEntropy_->HFEntropyCoeffGroupSet(mbX == 0, hfBandIdx_, colorComponent_, componentShiftX_, bestTransType_);
                        hfEncoderEntropy_->HFEntropyCoeffGroup(mbCoeff_, &hfEncoderEntropy_->bgParams_, hfTransformSkip);

                        hfIQuant_->Set((MB_SIZE >> componentShiftX_), MB_SIZE, curMbQP, HF_IQUANT_DYNAMIC_BIT);
                        hfIQuant_->ComIQuant(mbCoeff_, mbRec_, 0);

                        if (bestTransType_ == HF_HAD) {
                            hfTransITrans_->Set(componentShiftX_, 1);
                            hfTransITrans_->ComHFTransITrans(mbRec_, mbRec_);
                        }

                        hfMBRecReorder(bestTransType_);
                    }
                }
            }
        }

    }
}