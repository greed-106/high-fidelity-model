#include <memory>
#include "LLDecoder.h"

namespace ProVivid {
    LLDecoder::LLDecoder()
    {
        puPred_.resize(MB_SIZE * MB_SIZE);
        coeff_.resize(MB_SIZE * MB_SIZE);
        llPuIntra_ = std::make_shared<LLPuIntra>();
        llPuCclm_ = std::make_shared<LLPuCclm>();
        llPuITrans_ = std::make_shared<LLPuITrans>();
        llIQuant_ = std::make_shared<IQuant>();
        llPuRecon_ = std::make_shared<LLPuRecon>(llPuITrans_, llIQuant_);
        llPuInter_ = std::make_shared<LLPuInter>();
    }

    LLDecoder::~LLDecoder()
    {
    }

    void LLDecoder::GetPuWH(uint8_t puSize) {
        if (isChroma_ != LUMA) {
            puWidth_ = 4; puHeight_ = 8;
        } else if (puSize == LUMA_PU_4x4) {
            puWidth_ = 4; puHeight_ = 4;
        } else {
            puWidth_ = 8; puHeight_ = 8;
        }
    }

    void LLDecoder::Set(uint32_t llBandWidth, uint32_t llBandHeight, std::vector<SharedFrameBuffer>& llBandPic,
        uint8_t qp, int32_t cbQpOffset, int32_t crQpOffset) {
        llBandWidth_ = llBandWidth;
        llBandHeight_ = llBandHeight;
        llBandPic_ = &llBandPic;
        qp_[Y] = qp;
        Clip<int8_t, uint8_t>(qp_[Y] + cbQpOffset, qp_[U], 0, 39);
        Clip<int8_t, uint8_t>(qp_[Y] + crQpOffset, qp_[V], 0, 39);
    }

    void LLDecoder::Set(uint32_t llBandWidth, uint32_t llBandHeight, std::vector<SharedFrameBuffer>& llBandPic, std::vector<SharedFrameBuffer>& llBandPicRef,
        uint8_t qp, int32_t cbQpOffset, int32_t crQpOffset) {
        llBandWidth_ = llBandWidth;
        llBandHeight_ = llBandHeight;
        llBandPic_ = &llBandPic;
        llBandPicRef_ = &llBandPicRef;
        qp_[Y] = qp;
        Clip<int8_t, uint8_t>(qp_[Y] + cbQpOffset, qp_[U], 0, 39);
        Clip<int8_t, uint8_t>(qp_[Y] + crQpOffset, qp_[V], 0, 39);
    }

    void LLDecoder::LLDecode(Bitstream* bitstream, SeqPicHeaderInfo* seqPicHeaderInfo, SubpicSyntaxInfo* subpicSyntaxInfo)
    {
        auto llDecoderEntropy = std::make_shared<LLDecoderEntropy>(bitstream, subpicSyntaxInfo);
        int maxWidthMb = (llBandWidth_ + MB_SIZE - 1) / MB_SIZE;
        int maxHeightMb = (llBandHeight_ + MB_SIZE - 1) / MB_SIZE;

        uint8_t	refQP[3] = {0};
        for (uint32_t mbY = 0; mbY < maxHeightMb; mbY++) {
            for (uint32_t mbX = 0; mbX < maxWidthMb; mbX++) {
                if (mbX == 0) {
                    llDecoderEntropy->mbEntropyInfo_.leftPuMvdX = 0;
                    llDecoderEntropy->mbEntropyInfo_.leftPuMvdY = 0;
                }
                llDecoderEntropy->LLEntropyMbInfo(seqPicHeaderInfo->frameType, seqPicHeaderInfo->qpDeltaEnable);
                auto info = llDecoderEntropy->mbEntropyInfo_;
                uint8_t intraPredMode;
                uint8_t puSize = info.tuSizeLuma;

                int qpMbDelta = info.qpDelta;
                if (seqPicHeaderInfo->qpDeltaEnable) {
                    if (mbX == 0) {
                        for (int i = 0; i < 3; i++)
                        {
                            Clip<int8_t, uint8_t>(qpMbDelta + qp_[i], mbQp_[i], 0, 39);
                            refQP[i] = mbQp_[i];
                        }
                    } else
                    {
                        for (int i = 0; i < 3; i++)
                        {
                            Clip<int8_t, uint8_t>(qpMbDelta + refQP[i], mbQp_[i], 0, 39);
                            refQP[i] = mbQp_[i];
                        }
                    }
                } else
                {
                    for (int i = 0; i < 3; i++)
                    {
                        mbQp_[i] = qp_[i];
                    }
                }
                if (info.mbMode == MB_P) {
                    //P MB
                    MotionVector mv = MotionVector(info.leftPuMvdX, info.leftPuMvdY);
                    for (isChroma_ = LUMA; isChroma_ < LUMA_CHROMA; isChroma_++) {
                        lineWidth_ = llBandWidth_ >> isChroma_;
                        GetPuWH(info.tuSizeLuma);

                        for (colorComponent_ = isChroma_; colorComponent_ <= ((isChroma_ == LUMA) ? Y : V); colorComponent_++) {
                            puPixelIndex_ = (mbY * MB_SIZE) * lineWidth_ + mbX * (MB_SIZE >> isChroma_);
                            llPuInter_->Set(colorComponent_, puWidth_, puHeight_);
                            llPuInter_->GeRefWindow((*llBandPicRef_)[colorComponent_], lineWidth_, colorComponent_, mbY, mbX, mbY == 0, mbY == maxHeightMb - 1, mbX == 0, mbX == maxWidthMb - 1);
                            llPuInter_->InterPred(mv, puPred_, colorComponent_);
                            int idx = 0;
                            if (info.interNoResidualFlag) {
                                std::fill(coeff_.begin(), coeff_.end(), 0);
                            } else {
                                llDecoderEntropy->LLEntropyCoeff(colorComponent_, coeff_);
                            }
                            //IQIT
                            llPuRecon_->Set(puWidth_, puHeight_, puPixelIndex_, lineWidth_);
                            llPuRecon_->ComLLRecon(coeff_, puPred_, llBandPic_->at(colorComponent_), mbQp_[colorComponent_], N_PRED_PRED);
                        }
                    }
                } else {
                    for (isChroma_ = LUMA; isChroma_ < LUMA_CHROMA; isChroma_++) {
                        intraPredMode = (isChroma_ == LUMA) ? info.PredmodeLuma : info.PredmodeChroma;
                        lineWidth_ = llBandWidth_ >> isChroma_;
                        GetPuWH(info.tuSizeLuma);

                        for (colorComponent_ = isChroma_; colorComponent_ <= ((isChroma_ == LUMA) ? Y : V); colorComponent_++) {
                            for (puY_ = 0; puY_ < ((!isChroma_ && info.tuSizeLuma == LUMA_PU_4x4) ? 2 : 1); puY_++) {
                                for (puX_ = 0; puX_ < ((!isChroma_ && info.tuSizeLuma == LUMA_PU_4x4) ? 2 : 1); puX_++) {
                                    puPixelIndex_ = (mbY * MB_SIZE + puY_ * (MB_SIZE >> 1)) * lineWidth_ + mbX * (MB_SIZE >> isChroma_) + puX_ * (MB_SIZE >> 1);
                                    if (isChroma_ == CHROMA && intraPredMode == INTRA_CCLM) {
                                        llPuCclm_->Set(mbX, mbY, colorComponent_);
                                        uint32_t puYPixelIndex = mbY * MB_SIZE * (lineWidth_ << 1) + mbX * MB_SIZE;
                                        if (colorComponent_ == U) {
                                            llPuCclm_->GetCclmScale(llBandPic_->at(Y), llBandPic_->at(U), llBandPic_->at(V), puYPixelIndex, lineWidth_ << 1, puPixelIndex_, lineWidth_);
                                        }
                                        llPuCclm_->ComLLCclmPred(llBandPic_->at(Y), puYPixelIndex, lineWidth_ << 1, puPred_);
                                    } else {
                                        llPuIntra_->Set(puSize, intraPredMode, isChroma_, puX_, puY_, puWidth_, puHeight_, mbX, mbY, puPixelIndex_, lineWidth_);
                                        llPuIntra_->GetPuNbr(llBandPic_->at(colorComponent_));
                                        llPuIntra_->ComLLPred(puPred_);
                                    }

                                    llDecoderEntropy->LLEntropyCoeff(colorComponent_, coeff_);
                                    llPuRecon_->Set(puWidth_, puHeight_, puPixelIndex_, lineWidth_);
                                    llPuRecon_->ComLLRecon(coeff_, puPred_, llBandPic_->at(colorComponent_), mbQp_[colorComponent_], intraPredMode);
                                }
                            }
                        }
                    }
                }
            }
        }
    }

}