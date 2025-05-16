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

namespace ProVivid {
    Encoder::Encoder(std::string recFile, std::string recLLFile, uint32_t frameCount, uint32_t bitDepth, uint32_t intraPeriod)
        : recFile_(std::move(recFile)), recLLFile_(std::move(recLLFile)), frameCount_(frameCount), bitDepth_(bitDepth), intraPeriod_(intraPeriod)
    {
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

    Encoder::~Encoder()
    {
        if (recFileHandle_.is_open()) {
            recFileHandle_.close();
        }
        if (recLLFileHandle_.is_open()) {
            recLLFileHandle_.close();
        }
    }

    void Encoder::SetInput(std::shared_ptr<SubPicEnc> subPic, int32_t qp, int32_t cbQpOffset, int32_t crQpOffset, int32_t hlQpOffset, int32_t lhQpOffset, int32_t hhQpOffset,
        bool qpDeltaEnable, bool hfTransformSkip)
    {
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

    void Encoder::StreamCabacCtxIni()
    {
        // vlc stream
        BitstreamInit(&bitstreamVlcLl_, 24);
        BitstreamInit(&bitstreamVlcHf_, 24);
        // cabac ee and stream
        BitstreamInit(&bitstreamCabacLl_, 24);
        eeCabacLl_ = {0};
        eeCabacHf_ = {0};
        arienco_start_encoding(&eeCabacLl_, bitstreamCabacLl_.streamBuffer, &(bitstreamCabacLl_.byte_pos));
        arienco_reset_EC(&eeCabacLl_);
        BitstreamInit(&bitstreamCabacHf_, 24);
        arienco_start_encoding(&eeCabacHf_, bitstreamCabacHf_.streamBuffer, &(bitstreamCabacHf_.byte_pos));
        arienco_reset_EC(&eeCabacHf_);
        // ctx ini
        InitContextsLl(qp_, &texCtx_, &motCtx_);
        InitContextsHf(qp_, &highBandCtx_);
    }

    void Encoder::SubpicEncodingDone(uint32_t subpicIndex)
    {
        //stuffing vlc
        WriteStuffingBits(&bitstreamVlcLl_);
        WriteStuffingBits(&bitstreamVlcHf_);
        // terminate the arithmetic code
        biari_encode_symbol_final(&eeCabacLl_, 1);
        arienco_done_encoding(&eeCabacLl_);
        biari_encode_symbol_final(&eeCabacHf_, 1);
        arienco_done_encoding(&eeCabacHf_);
        //write qp info/subpic size
        
        //qp to be added
        int subPicBytes = bitstreamVlcLl_.byte_pos + bitstreamVlcHf_.byte_pos + *eeCabacLl_.Ecodestrm_len + *eeCabacHf_.Ecodestrm_len;
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
        picSubpicInfoPointer[8] = *eeCabacLl_.Ecodestrm_len >> 16;
        picSubpicInfoPointer[9] = (*eeCabacLl_.Ecodestrm_len >> 8) & 0xff;
        picSubpicInfoPointer[10] = *eeCabacLl_.Ecodestrm_len & 0xff;
        picSubpicInfoPointer[11] = bitstreamVlcLl_.byte_pos >> 16;
        picSubpicInfoPointer[12] = (bitstreamVlcLl_.byte_pos >> 8) & 0xff;
        picSubpicInfoPointer[13] = bitstreamVlcLl_.byte_pos & 0xff;
        picSubpicInfoPointer[14] = *eeCabacHf_.Ecodestrm_len >> 16;
        picSubpicInfoPointer[15] = (*eeCabacHf_.Ecodestrm_len >> 8) & 0xff;
        picSubpicInfoPointer[16] = *eeCabacHf_.Ecodestrm_len & 0xff;

        memcpy(bitstream_.streamBuffer + bitstream_.byte_pos , eeCabacLl_.Ecodestrm, *eeCabacLl_.Ecodestrm_len);
        memcpy(bitstream_.streamBuffer + bitstream_.byte_pos + *eeCabacLl_.Ecodestrm_len, bitstreamVlcLl_.streamBuffer, bitstreamVlcLl_.byte_pos);
        memcpy(bitstream_.streamBuffer + bitstream_.byte_pos + *eeCabacLl_.Ecodestrm_len + bitstreamVlcLl_.byte_pos, eeCabacHf_.Ecodestrm, *eeCabacHf_.Ecodestrm_len);
        memcpy(bitstream_.streamBuffer + bitstream_.byte_pos + *eeCabacLl_.Ecodestrm_len + bitstreamVlcLl_.byte_pos + *eeCabacHf_.Ecodestrm_len, bitstreamVlcHf_.streamBuffer, bitstreamVlcHf_.byte_pos);

        BitstreamRelease(&bitstreamCabacLl_);
        BitstreamRelease(&bitstreamCabacHf_);
        BitstreamRelease(&bitstreamVlcLl_);
        BitstreamRelease(&bitstreamVlcHf_);

        bitstream_.byte_pos += subPicBytes;
    }

    void Encoder::PicEncodingDone()
    {
        int frameByte = bitstream_.byte_pos - seqHeaderBytes_;
        LOGI("BYTES %d\n", bitstream_.byte_pos);
        byte *picSizePointer = bitstream_.streamBuffer + seqHeaderBytes_;
        picSizePointer[0] = frameByte >> 24;
        picSizePointer[1] = (frameByte >> 16) & 0xff;
        picSizePointer[2] = (frameByte >> 8) & 0xff;
        picSizePointer[3] = frameByte & 0xff;
    }


    void Encoder::Encode(uint32_t currFrame)
    {
        auto frameType = static_cast<FrameType>(currFrame % intraPeriod_);
        bool needRef = frameType == FRAME_I && frameCount_ > 1 && intraPeriod_ > 1;
        auto llEncoder = std::make_shared<LLEncoder>();
        auto hfEncoder = std::make_shared<HFEncoder>();
        for (auto & subPicInfo : subPic_->subPicInfo_) {
            uint32_t subPicId = subPicInfo[Y].id;
            StreamCabacCtxIni();
            subPic_->DWT(subPicInfo);
            if (frameType == FRAME_P) {
                subPic_->SetLLReference(subPicLLInfoRef_[subPicId]);
                llEncoder->Set(subPicInfo[Y].w >> 1, subPicInfo[Y].h >> 1,
                    subPic_->subBands_[LL], subPic_->subBandsRec_[LL], subPic_->subBandsRef_[LL], 
                    qp_, cbQpOffset_, crQpOffset_);
            }
            else {
                llEncoder->Set(subPicInfo[Y].w >> 1, subPicInfo[Y].h >> 1,
                    subPic_->subBands_[LL], subPic_->subBandsRec_[LL], qp_, cbQpOffset_, crQpOffset_);
            }
            llEncoder->LLEncode(&bitstreamVlcLl_, &eeCabacLl_, &texCtx_, &motCtx_, frameType, qpDeltaEnable_);

            hfEncoder->Set(subPicInfo[Y].w >> 1, subPicInfo[Y].h >> 1,
                subPic_->subBands_, subPic_->subBandsRec_,
                qp_, cbQpOffset_, crQpOffset_, hlQpOffset_, lhQpOffset_, hhQpOffset_);
            hfEncoder->HFEncode(&bitstreamVlcHf_, &eeCabacHf_, &highBandCtx_, qpDeltaEnable_, hfTransformSkip_);

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
