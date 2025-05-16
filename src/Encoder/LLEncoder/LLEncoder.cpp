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
#include "LLEncoder.h"

namespace ProVivid {
    LLEncoder::LLEncoder() {
        puPred_.resize(MB_SIZE * MB_SIZE);
        predResidual_.resize(MB_SIZE * MB_SIZE);
        coeff_.resize(MB_SIZE * MB_SIZE);
        pBestYRec_ = std::make_shared<FrameBuffer>(MB_SIZE * MB_SIZE, 0);
        llPuIntra_ = std::make_shared<LLPuIntra>();
        llPuInter_ = std::make_shared<LLEncoderME>();
        llPuCclm_ = std::make_shared<LLPuCclm>();
        llEncoderTrans_ = std::make_shared<LLEncoderTrans>();
        encoderQuant_ = std::make_shared<EncoderQuant>();
        llPuITrans_ = std::make_shared<LLPuITrans>();
        llIQuant_ = std::make_shared<IQuant>();
        llPuRecon_ = std::make_shared<LLPuRecon>(llPuITrans_, llIQuant_);
        llEncoderEntropy_ = std::make_shared<LLEncoderEntropy>();
    }

    LLEncoder::~LLEncoder() {
    }

    void LLEncoder::Set(uint32_t llBandWidth, uint32_t llBandHeight, std::vector<SharedFrameBuffer>& llBandPic, std::vector<SharedFrameBuffer>& llBandPicRec,
        uint8_t qp, int32_t cbQpOffset, int32_t crQpOffset) {
        llBandWidth_ = llBandWidth;
        llBandHeight_ = llBandHeight;
        llBandPic_ = &llBandPic;
        llBandPicRec_ = &llBandPicRec;
        qp_[Y] = qp;
        Clip<int8_t, uint8_t>(qp_[Y] + cbQpOffset, qp_[U], 0, 39);
        Clip<int8_t, uint8_t>(qp_[Y] + crQpOffset, qp_[V], 0, 39);
    }

    void LLEncoder::Set(uint32_t llBandWidth, uint32_t llBandHeight, std::vector<SharedFrameBuffer>& llBandPic, std::vector<SharedFrameBuffer>& llBandPicRec, std::vector<SharedFrameBuffer>& llBandPicRef,
        uint8_t qp, int32_t cbQpOffset, int32_t crQpOffset) {
        llBandWidth_ = llBandWidth;
        llBandHeight_ = llBandHeight;
        llBandPic_ = &llBandPic;
        llBandPicRec_ = &llBandPicRec;
        llBandPicRef_ = &llBandPicRef;
        qp_[Y] = qp;
        Clip<int8_t, uint8_t>(qp_[Y] + cbQpOffset, qp_[U], 0, 39);
        Clip<int8_t, uint8_t>(qp_[Y] + crQpOffset, qp_[V], 0, 39);
        llPuInter_->SetQP(qp, cbQpOffset, crQpOffset);
    }

    void LLEncoder::GetPuWH(uint8_t puSize) {
        if (isChroma_ != LUMA) {
            puWidth_ = 4; puHeight_ = 8;
        } else if (puSize == LUMA_PU_4x4) {
            puWidth_ = 4; puHeight_ = 4;
        } else {
            puWidth_ = 8; puHeight_ = 8;
        }
    }


    void LLEncoder::Residual() {
        uint32_t curIndex = 0;
        uint32_t picIndex = puPixelIndex_;
        for (uint8_t yPos = 0; yPos < puHeight_; ++yPos) {
            for (uint8_t xPos = 0; xPos < puWidth_; ++xPos) {
                predResidual_[curIndex] = llBandPic_->at(colorComponent_)->at(picIndex + xPos) - puPred_[curIndex];
                curIndex++;
            }
            picIndex += lineWidth_;
        }
    }

    void LLEncoder::LLEncodePu(uint8_t puSize, bool interPredFlag, uint8_t predMode, bool withResi) {
        if (withResi) {
            Residual();
            llEncoderTrans_->Set(puWidth_, puHeight_, predMode);
            llEncoderTrans_->LLTrans(predResidual_, coeff_);

            uint8_t curMbQP = mbQp_[colorComponent_];
            encoderQuant_->Set(puWidth_, puHeight_, curMbQP, LL_DEADZONE, LL_TH1, LL_RDOQ_OFFSET, LL_QUANT_DYNAMIC_BIT);
            encoderQuant_->Quant(coeff_, coeff_, (puWidth_ >> 3) + (puHeight_ >> 3));
            llEncoderEntropy_->LLEntropyCoeff(colorComponent_, puSize, predMode, coeff_);

            llPuRecon_->Set(puWidth_, puHeight_, puPixelIndex_, lineWidth_);
            llPuRecon_->ComLLRecon(coeff_, puPred_, llBandPicRec_->at(colorComponent_), curMbQP, predMode);
        } else {
            llPuRecon_->Set(puWidth_, puHeight_, puPixelIndex_, lineWidth_);
            llPuRecon_->ComLLRecon(puPred_, llBandPicRec_->at(colorComponent_));
        }
    };


    void LLEncoder::LLEncodeIntraPu(uint8_t puSize, uint8_t intraPredMode) {
        if (isChroma_ == CHROMA && intraPredMode == INTRA_CCLM) {
            llPuCclm_->Set(mbX_, mbY_, colorComponent_);
            uint32_t puYPixelIndex = mbY_ * MB_SIZE * (lineWidth_ << 1) + mbX_ * MB_SIZE;
            if (colorComponent_ == U) {
                llPuCclm_->GetCclmScale(llBandPicRec_->at(Y), llBandPicRec_->at(U), llBandPicRec_->at(V), puYPixelIndex, lineWidth_ << 1, puPixelIndex_, lineWidth_);
            }
            llPuCclm_->ComLLCclmPred(pBestYRec_, 0, MB_SIZE, puPred_);
        } else {
            llPuIntra_->Set(puSize, intraPredMode, isChroma_, puX_, puY_, puWidth_, puHeight_, mbX_, mbY_, puPixelIndex_, lineWidth_);
            llPuIntra_->GetPuNbr(llBandPicRec_->at(colorComponent_));
            llPuIntra_->ComLLPred(puPred_);
        }

        LLEncodePu(puSize, 0, intraPredMode, 1);
    };

    void LLEncoder::LLEncodeInterPu(MotionVector mv, bool withResi) {
        llPuInter_->Set(colorComponent_, puWidth_, puHeight_);
        llPuInter_->InterPred(mv, puPred_, colorComponent_);
        LLEncodePu(puSize_, 1, N_PRED_PRED, withResi);
    };

    uint32_t LLEncoder::Distortion() {
        uint32_t sse = 0;
        uint32_t tmp = 0;
        uint32_t picIndex = puPixelIndex_;
        for (uint8_t yPos = 0; yPos < puHeight_; ++yPos) {
            for (uint8_t xPos = 0; xPos < puWidth_; ++xPos) {
                tmp = llBandPic_->at(colorComponent_)->at(picIndex + xPos) - llBandPicRec_->at(colorComponent_)->at(picIndex + xPos);
                sse += tmp * tmp;
            }
            picIndex += lineWidth_;
        }
        return sse;
    }

    void LLEncoder::IntraRdCost() {
        float curRdCost = RdCostCal(rate_, dis_, mbQp_[isChroma_]);
        if (curRdCost < bestIntraRdcost_[isChroma_]) {
            bestIntraRdcost_[isChroma_] = curRdCost;
            if (isChroma_ == LUMA) {
                bestLumaPuSize_ = puSize_;
                uint32_t picIndex = mbY_ * MB_SIZE  * lineWidth_ + mbX_ * MB_SIZE;
                for (int h = 0; h < MB_SIZE; h++) {
                    for (int w = 0; w < MB_SIZE; w++) {
                        pBestYRec_->at(h * MB_SIZE + w) = llBandPicRec_->at(Y)->at(picIndex + w);
                    }
                    picIndex += lineWidth_;
                }
            }
            bestIntraPredMode_[isChroma_] = intraPredMode_;
        }
    }

    void LLEncoder::InterRdCost(float curRdCost) {
        if (curRdCost < bestInterRdcost_) {
            bestInterRdcost_ = curRdCost;
            bestMVDInfo_ = MVDInfo_;
            bestResInfo_ = resInfo_;
            bestMv_ = mv_;
        }
    }

    void LLEncoder::LLEncode(Bitstream* bitstreamVlcLl, EncodingEnvironment* eeCabacLl, TextureInfoContexts* texCtx, MotionInfoContexts* motCtx, 
        int frameType, bool qpDeltaEnable, std::vector<std::vector<int16_t>>& mbDeltaQP) {
        llEncoderEntropy_->Set(bitstreamVlcLl, eeCabacLl, texCtx, motCtx);

        int maxWidthMb = (llBandWidth_ + MB_SIZE - 1) / MB_SIZE;
        int maxHeightMb = (llBandHeight_ + MB_SIZE - 1) / MB_SIZE;
        uint8_t  refQP_U = 0;
        uint8_t  refQP_V = 0;
        for (mbY_ = 0; mbY_ < maxHeightMb; mbY_++) {
            //printf(" \n");
            for (mbX_ = 0; mbX_ < maxWidthMb; mbX_++) {
                //auto llEncoderMB = std::make_shared<std::vector<LLEncoderMB>>(N_pu_size * N_INTRA_PRED);
                if (qpDeltaEnable) {
                    int curMbQP; //to be replaced by AQ
                    int qpMbDelta; //to be replaced by AQ
                    uint32_t pos = (mbY_ * MB_SIZE) * llBandWidth_ + mbX_ * MB_SIZE;
                    Clip(qp_[Y] + mbDeltaQP[mbY_][mbX_], curMbQP, 0, 39);
                    llEncoderEntropy_->LLEntropyDeltaQp(mbX_ == 0, qp_[Y], curMbQP, qpMbDelta);
                    mbQp_[Y] = curMbQP;
                    //printf(" \t %d", curMbQP);
                    if (mbX_ == 0) {
                        Clip<uint8_t, uint8_t>(qpMbDelta + qp_[V], mbQp_[V], 0, 39);
                        Clip<uint8_t, uint8_t>(qpMbDelta + qp_[U], mbQp_[U], 0, 39);
                        refQP_U = mbQp_[U];
                        refQP_V = mbQp_[V];
                    } else {
                        Clip<uint8_t, uint8_t>(qpMbDelta + refQP_V, mbQp_[V], 0, 39);
                        Clip<uint8_t, uint8_t>(qpMbDelta + refQP_U, mbQp_[U], 0, 39);
                        refQP_U = mbQp_[U];
                        refQP_V = mbQp_[V];
                    }
                } else {
                    mbQp_[Y] = qp_[Y];
                    mbQp_[U] = qp_[U];
                    mbQp_[V] = qp_[V];
                }
                if (frameType == FRAME_P && mbX_ == 0) {
                    preMv_ = MotionVector(0, 0);
                }
                bestMbInter_ = MB_I;
                //intra
                bestLumaPuSize_ = 0;
                for (isChroma_ = LUMA; isChroma_ < LUMA_CHROMA; isChroma_++) {
                    bestIntraRdcost_[isChroma_] = static_cast<float>(INT32_MAX);
                    bestIntraPredMode_[isChroma_] = INTRA_VER;
                    lineWidth_ = llBandWidth_ >> isChroma_;
                    //puPixelIndex_ = mbX_ * MB_SIZE * lineWidth_ + mbY_ * (MB_SIZE >> componentShiftX);
                    //llPUIntra->GetPUNbr(llBandPicRec_, llBandWidth_, mbX, mbY);
                    for (puSize_ = LUMA_PU_4x4; puSize_ <= ((isChroma_ == LUMA) ? LUMA_PU_8x8 : CHROMA_PU_4x8); puSize_++) {
                        GetPuWH(puSize_);
                        for (intraPredMode_ = INTRA_VER; intraPredMode_ < N_PRED_PRED; intraPredMode_++) {
#if !PLANAR
                            if (isChroma_ == LUMA && intraPredMode_ == INTRA_PLANAR) {
                                continue;
                            }
#endif
#if !CCLM
                            if (isChroma_ == CHROMA && intraPredMode_ == INTRA_CCLM) {
                                continue;
                            }
#endif
                            //start rdo
                            rate_ = 0; dis_ = 0;
                            llEncoderEntropy_->GetCabcaState();
                            llEncoderEntropy_->LLEntropyMbMode(frameType, MB_I); //0: I mb, 1:P mb
                            for (colorComponent_ = isChroma_; colorComponent_ <= ((isChroma_ == LUMA) ? Y : V); colorComponent_++) {
                                for (puY_ = 0; puY_ < ((!isChroma_ && puSize_ == LUMA_PU_4x4) ? 2 : 1); puY_++) {
                                    for (puX_ = 0; puX_ < ((!isChroma_ && puSize_ == LUMA_PU_4x4) ? 2 : 1); puX_++) {
                                        puPixelIndex_ = (mbY_ * MB_SIZE + puY_ * (MB_SIZE >> 1)) * lineWidth_ + mbX_ * (MB_SIZE >> isChroma_) + puX_ * (MB_SIZE >> 1);
                                        LLEncodeIntraPu(puSize_, intraPredMode_);
                                        dis_ += Distortion();
                                    }
                                }
                            }
                            llEncoderEntropy_->LLEntropyPuSize(1, isChroma_, puSize_);
                            llEncoderEntropy_->LLEntropyIntraPredMode(isChroma_, intraPredMode_);
                            rate_ += llEncoderEntropy_->bits_;
                            IntraRdCost();
                            llEncoderEntropy_->ResetCabcaState();
                        }
                    }
                }

                //inter prediction
                bestInterRdcost_ = static_cast<float>(INT32_MAX);
                if (frameType == FRAME_P) {
                    // fetch reference pixels
                    for (uint32_t compId = 0; compId < N_COLOR_COMP; compId++) {
                        uint8_t componentShiftX = 0;
                        if (compId != Y) {
                            componentShiftX = 1;
                        }
                        lineWidth_ = llBandWidth_ >> componentShiftX;
                        uint32_t puWidth = MB_SIZE >> componentShiftX;
                        uint32_t puHeight = MB_SIZE;
                        uint32_t puPos = (mbY_ * MB_SIZE) * lineWidth_ + mbX_ * (MB_SIZE >> componentShiftX);
                        llPuInter_->Set(compId, puWidth, puHeight);
                        llPuInter_->SetOrg(compId, llBandPic_->at(compId), puPos, lineWidth_);
                        llPuInter_->GeRefWindow((*llBandPicRef_)[compId], lineWidth_, compId, mbY_, mbX_, mbY_ == 0, mbY_ == maxHeightMb - 1, mbX_ == 0, mbX_ == maxWidthMb - 1);
                    }
                    if (mbX_) {
                        llPuInter_->SetMVP(preMv_);
                    } else {
                        llPuInter_->SetMVP(MotionVector());
                    }

                    float bestMECost = static_cast<float>(INT32_MAX);
                    MotionVector intMv = llPuInter_->IntPxlME(bestMECost);
                    MotionVector fracMv = llPuInter_->HalfPxlME(intMv, bestMECost);
                    float curRdCost;
                    bool mvEq = (fracMv.mvX == preMv_.mvX) & (fracMv.mvY == preMv_.mvY);

                    for (int32_t withMVD = 0; withMVD <= (mvEq ? 0 : 1); withMVD++) {
                        for (uint32_t withResi = 0; withResi <= 1; withResi++) {
                            rate_ = 0; dis_ = 0; curRdCost = 0;
                            llEncoderEntropy_->GetCabcaState();
                            MVDInfo_ = withMVD;
                            resInfo_ = withResi;
                            mv_ = withMVD ? fracMv : preMv_;
                            llEncoderEntropy_->LLEntropyMbMode(frameType, MB_P); //0: I mb, 1:P mb
                            llEncoderEntropy_->LLEntropyInterMode(!withResi);
                            llEncoderEntropy_->LLEntropyMvdMode(withMVD);
                            if (withMVD) {
                                llEncoderEntropy_->LLEntropyMvd(fracMv.mvX - preMv_.mvX, preMv_.mvX, 0, fracMv.mvX - preMv_.mvX);
                                llEncoderEntropy_->LLEntropyMvd(fracMv.mvY - preMv_.mvY, preMv_.mvY, 1, fracMv.mvX - preMv_.mvX);
                            }
                            uint32_t rate_tmp;
                            for (isChroma_ = LUMA; isChroma_ < LUMA_CHROMA; isChroma_++) {
                                rate_tmp = rate_;
                                dis_ = 0;
                                lineWidth_ = llBandWidth_ >> isChroma_;
                                puSize_ = (isChroma_ == LUMA) ? LUMA_PU_8x8 : CHROMA_PU_4x8;
                                GetPuWH(puSize_);
                                puPixelIndex_ = (mbY_ * MB_SIZE) * lineWidth_ + mbX_ * (MB_SIZE >> isChroma_);
                                for (colorComponent_ = isChroma_; colorComponent_ <= ((isChroma_ == LUMA) ? Y : V); colorComponent_++) {
                                    if (withMVD == 0) {
                                        LLEncodeInterPu(preMv_, withResi);
                                    } else {
                                        LLEncodeInterPu(fracMv, withResi);
                                    }
                                    dis_ += Distortion();
                                }
                                rate_ = llEncoderEntropy_->bits_;
                                curRdCost += RdCostCal(rate_ - rate_tmp, dis_, mbQp_[isChroma_]);
                            }
                            InterRdCost(curRdCost);
                            llEncoderEntropy_->ResetCabcaState();
                        }
                    }
                }

                bestMbInter_ = bestInterRdcost_ < (bestIntraRdcost_[LUMA] + bestIntraRdcost_[CHROMA]);

                //real encode
                if (bestMbInter_) {
                    //printf("(%d,%d) withMVD: %d, withResi:%d, MV:(%d,%d) MVP:(%d,%d)\n", mbX_, mbY_, bestMVDInfo_, bestResInfo_, bestMv_.mvX, bestMv_.mvY, preMv_.mvX, preMv_.mvY);
                    llEncoderEntropy_->LLEntropyMbMode(frameType, MB_P); //0: I mb, 1:P mb
                    //llEncoderEntropy_->inter;
                    llEncoderEntropy_->LLEntropyInterMode(!bestResInfo_);
                    llEncoderEntropy_->LLEntropyMvdMode(bestMVDInfo_);
                    if (bestMVDInfo_) {
                        llEncoderEntropy_->LLEntropyMvd(bestMv_.mvX - preMv_.mvX, bestMv_.mvX, 0, bestMv_.mvX - preMv_.mvX);
                        llEncoderEntropy_->LLEntropyMvd(bestMv_.mvY - preMv_.mvY, bestMv_.mvY, 1, bestMv_.mvX - preMv_.mvX);
                    }

                    for (isChroma_ = LUMA; isChroma_ < LUMA_CHROMA; isChroma_++) {
                        lineWidth_ = llBandWidth_ >> isChroma_;
                        puSize_ = (isChroma_ == LUMA) ? LUMA_PU_8x8 : CHROMA_PU_4x8;
                        GetPuWH(puSize_);
                        puPixelIndex_ = (mbY_ * MB_SIZE) * lineWidth_ + mbX_ * (MB_SIZE >> isChroma_);
                        for (colorComponent_ = isChroma_; colorComponent_ <= ((isChroma_ == LUMA) ? Y : V); colorComponent_++) {
                            LLEncodeInterPu(bestMv_, bestResInfo_);
                        }
                    }
                    preMv_ = bestMv_;
                } else {
                    llEncoderEntropy_->LLEntropyMbMode(frameType, MB_I); //0: I mb, 1:P mb
                    llEncoderEntropy_->LLEntropyPuSize(1, LUMA, bestLumaPuSize_);
                    llEncoderEntropy_->LLEntropyIntraPredMode(LUMA, bestIntraPredMode_[LUMA]);
                    llEncoderEntropy_->LLEntropyIntraPredMode(CHROMA, bestIntraPredMode_[CHROMA]);

                    for (isChroma_ = LUMA; isChroma_ < LUMA_CHROMA; isChroma_++) {
                        lineWidth_ = llBandWidth_ >> isChroma_;
                        GetPuWH(bestLumaPuSize_);
                        for (colorComponent_ = isChroma_; colorComponent_ <= ((isChroma_ == LUMA) ? Y : V); colorComponent_++) {
                            for (puY_ = 0; puY_ < ((!isChroma_ && bestLumaPuSize_ == LUMA_PU_4x4) ? 2 : 1); puY_++) {
                                for (puX_ = 0; puX_ < ((!isChroma_ && bestLumaPuSize_ == LUMA_PU_4x4) ? 2 : 1); puX_++) {
                                    puPixelIndex_ = (mbY_ * MB_SIZE + puY_ * (MB_SIZE >> 1)) * lineWidth_ + mbX_ * (MB_SIZE >> isChroma_) + puX_ * (MB_SIZE >> 1);
                                    LLEncodeIntraPu(bestLumaPuSize_, bestIntraPredMode_[isChroma_]);
                                }
                            }
                        }
                    }
                    preMv_ = MotionVector(0, 0);
                }

            }
        }
    }
}