#include "HFEncoder.h"

namespace ProVivid {
    HFEncoder::HFEncoder()
    {
        mbOri_.resize(MB_SIZE*MB_SIZE);
        mbHad_.resize(MB_SIZE*MB_SIZE);
        mbCoeff_.resize(MB_SIZE*MB_SIZE);
        mbRec_.resize(MB_SIZE*MB_SIZE);
        encoderQuant_ = std::make_shared<EncoderQuant>();
        hfIQuant_ = std::make_shared<IQuant>();
        hfTransITrans_ = std::make_shared<HFTransITrans>();
        hfEncoderEntropy_ = std::make_shared<HFEncoderEntropy>();
    }

    HFEncoder::~HFEncoder()
    {
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


    int32_t HFEncoder::DetermineMBdeltaQP(uint8_t subPicQP, uint32_t pos, uint32_t stride) {
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
                pixel = (*subPic_)[LL][Y]->at(picIndex + xPos);
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
                } else
                {
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
            int	varA = var8x8;
            if ((varMax - varMin) > (50000 >> 2))
                varA = varMin << 2;
            int varDiffAbs = (varA - 4096);
            int signFlag = varA < 4096 ? -1 : 1;
            int varStrengthTemp = 2;
            int varShiftTemp = 2;

            if (signFlag > 0)
            {
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
            if (subPicQP > 12)
                qpOffset = (subPicQP - 12) / 2;
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
            Clip(subPicQP + mbQpDeltaA + mbQpDeltaB, curMbQP, 0, 39);
            //curMbQP = qp_[Y] + mbQpDeltaA + mbQpDeltaB + mbQpDeltaC;
            if (curMbQP > 8)
                mbQpDeltaC = 8 - subPicQP - mbQpDeltaA - mbQpDeltaB;
        }

        return mbQpDeltaA + mbQpDeltaB + mbQpDeltaC;
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
        if (qpIndex < 16)
        {
            leftShift = 2 - (qpIndex >> 3);
        }

        if (qpIndex > 23)
        {
            rightShift = (qpIndex >> 3) - 2;
        }
        qpIndex &= 7;

        int qScale = SIMP_QUANT_SCALE[qpIndex];
        int qShift = SIMP_QUANT_SHIFT[qpIndex];
        int deadzone = SIMP_DEADZONE[qpIndex];
        int round = SIMP_ROUND[qpIndex];
        int quantsize = 1 << qShift;
        int quantsizeMin1 = quantsize - 1;

        for (int i = 0; i < (64); i++)
        {
            coeff = abs(mbPix[i]);
            coeff <<= leftShift;
            coeff >>= rightShift;

            coeff *= qScale;
            quantLeft = (coeff & quantsizeMin1);
            tempPix = coeff >> qShift;
            if (coeff > deadzone && quantLeft > round)
            {
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

    void HFEncoder::HFEncode(Bitstream* bitstreamVlcHf_, EncodingEnvironment* eeCabacHf_, HighBandInfoContexts* highBandCtx_, bool qpDeltaEnable, bool hfTransformSkip)
    {
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
                    Clip(qp_[HL][Y] + DetermineMBdeltaQP(qp_[HL][Y], pos, hfBandWidth_), curMbQP, 0, 39);
                    hfEncoderEntropy_->HFEntropyDeltaQp(mbX == 0, qp_[HL][Y], curMbQP, qpMBdelta);

                    for (int bandidx = HL; bandidx <= HH; bandidx++) {
                        for (int comIdx = Y; comIdx <= V; comIdx++) {
                            if (mbX == 0) {
                                Clip(qp_[bandidx][comIdx] + qpMBdelta, mbQp_[bandidx][comIdx], 0, 39);
                                refQP[bandidx][comIdx] = mbQp_[bandidx][comIdx];
                            } else
                            {
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
                        bestTransType_ = ((colorComponent_ == Y && SimpleRd(curMbQP, mbOri_) < SimpleRd(curMbQP, mbHad_)) ? HF_NOHAD : HF_HAD);
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