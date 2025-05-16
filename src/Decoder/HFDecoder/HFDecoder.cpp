#include "HFDecoder.h"

namespace ProVivid {
    HFDecoder::HFDecoder()
    {
        hfIQuant_ = std::make_shared<IQuant>();
        hfTransITrans_ = std::make_shared<HFTransITrans>();
        hfDecoderEntropy_ = std::make_shared<HFDecoderEntropy>();
        mbDec_.resize(MB_SIZE*MB_SIZE);
        mbCoeff_.resize(MB_SIZE*MB_SIZE);
    }

    HFDecoder::~HFDecoder()
    {
    }

    void HFDecoder::Set(uint32_t hfBandWidth, uint32_t hfBandHeight, SubBandMap& subPicDec,
        uint8_t qp, int32_t cbQpOffset, int32_t crQpOffset, int32_t hlQpOffset, int32_t lhQpOffset, int32_t hhQpOffset) {
        hfBandWidth_ = hfBandWidth;
        hfBandHeight_ = hfBandHeight;
        subPicDec_ = &subPicDec;
        Clip<int8_t, uint8_t>(qp + hlQpOffset, qp_[HL][Y], 0, 39);
        Clip<int8_t, uint8_t>(qp + lhQpOffset, qp_[LH][Y], 0, 39);
        Clip<int8_t, uint8_t>(qp + hhQpOffset, qp_[HH][Y], 0, 39);
        for (hfBandIdx_ = HL; hfBandIdx_ <= HH; hfBandIdx_++) {
            Clip<int8_t, uint8_t>(qp_[hfBandIdx_][Y] + cbQpOffset, qp_[hfBandIdx_][U], 0, 39);
            Clip<int8_t, uint8_t>(qp_[hfBandIdx_][Y] + crQpOffset, qp_[hfBandIdx_][V], 0, 39);
        }
    }

    void HFDecoder::hfMBDecReorder() {
        for (uint8_t i = 0; i < (MB_SIZE*MB_SIZE >> componentShiftX_); i++) {
            (*subPicDec_)[hfBandIdx_][colorComponent_]->at(pixelIndex_ + lineWidth_ * HF_REMAP_LINE[i] + HF_REMAP_POS_16[i / 32 * 8 + i % 8]) = mbDec_[i];
        }
    }

    void HFDecoder::HFDecode(Bitstream* bitstream, SeqPicHeaderInfo* seqPicHeaderInfo, SubpicSyntaxInfo* subpicSyntaxInfo)
    {
        hfDecoderEntropy_->Set(bitstream, subpicSyntaxInfo);
        int maxWidthMb = (hfBandWidth_ + MB_SIZE - 1) / MB_SIZE;
        int maxHeightMb = (hfBandHeight_ + MB_SIZE - 1) / MB_SIZE;

        FrameBuffer* ptr;
        uint8_t transType;
        uint8_t	refQP[N_SUB_BANDS][N_COLOR_COMP] = {0};
        uint8_t	mbQP[N_SUB_BANDS][N_COLOR_COMP] = {0};
        for (uint32_t mbY = 0; mbY < maxHeightMb; mbY++) {
            for (uint32_t mbX = 0; mbX < maxWidthMb; mbX++) {
                for (hfBandIdx_ = HL; hfBandIdx_ < N_SUB_BANDS; hfBandIdx_++) {
                    for (colorComponent_ = Y; colorComponent_ < N_COLOR_COMP; ++colorComponent_) {
                        componentShiftX_ = (colorComponent_ == Y) ? 0 : 1;
                        lineWidth_ = hfBandWidth_ >> componentShiftX_;
                        pixelIndex_ = mbY * MB_SIZE  * lineWidth_ + mbX * (MB_SIZE >> componentShiftX_);

                        std::fill(mbCoeff_.begin(), mbCoeff_.end(), 0);
                        hfDecoderEntropy_->HFEntropyCoeffGroupSet(mbX == 0, hfBandIdx_, colorComponent_);
                        hfDecoderEntropy_->HFEntropyDecode(seqPicHeaderInfo->qpDeltaEnable, seqPicHeaderInfo->hfTransformSkip, mbCoeff_);

                        //IQIT
                        uint8_t mb_qp_hf = 0;
                        if (seqPicHeaderInfo->qpDeltaEnable) {
                            if ((hfBandIdx_ == HL) && (colorComponent_ == Y)) {
                                int qpMbDelta = hfDecoderEntropy_->qpDelta;
                                for (int bandidx = HL; bandidx <= HH; bandidx++) {
                                    for (int comIdx = Y; comIdx <= V; comIdx++) {
                                        if (mbX == 0) {
                                            Clip<uint8_t, uint8_t>(qp_[bandidx][comIdx] + qpMbDelta, mbQP[bandidx][comIdx], 0, 39);
                                            refQP[bandidx][comIdx] = mbQP[bandidx][comIdx];
                                        } else
                                        {
                                            Clip<uint8_t, uint8_t>(qpMbDelta + refQP[bandidx][comIdx], mbQP[bandidx][comIdx], 0, 39);
                                            refQP[bandidx][comIdx] = mbQP[bandidx][comIdx];
                                        }
                                    }
                                }
                            }
                            mb_qp_hf = mbQP[hfBandIdx_][colorComponent_];
                        } else
                        {
                            mb_qp_hf = qp_[hfBandIdx_][colorComponent_];
                        }

                        hfIQuant_->Set((MB_SIZE >> componentShiftX_), MB_SIZE, mb_qp_hf, HF_IQUANT_DYNAMIC_BIT);
                        hfIQuant_->ComIQuant(mbCoeff_, mbDec_, 0);
                        transType = hfDecoderEntropy_->bgParams_.transformType;
                        if (transType == HF_HAD) {
                            hfTransITrans_->Set(colorComponent_, 1);
                            hfTransITrans_->ComHFTransITrans(mbDec_, mbDec_);
                        }
                        hfMBDecReorder();
                    }

                }
            }
        }

    }
}