#ifndef HF_ENCODER_H
#define HF_ENCODER_H

#include <vector>
#include "FrameBuffer.h"
#include "Const.h"
#include "Table.h"
#include "EncoderQuant.h"
#include "HFEncoderEntropy.h"
#include "IQuant.h"
#include "HFTransITrans.h"
#include <cstring>

namespace ProVivid {
    class HFEncoder {
    public:
        HFEncoder();
        ~HFEncoder();
        void Set(uint32_t hfBandWidth, uint32_t hfBandHeight, SubBandMap& subPic, SubBandMap& subPicRec,
            uint8_t qp, int32_t cbQpOffset, int32_t crQpOffset, int32_t hlQpOffset, int32_t lhQpOffset, int32_t hhQpOffset);
        uint32_t SimpleRd(uint8_t qp, FrameBuffer &mbPix);
        void HFEncode(Bitstream* bitstreamVlcHf_, EncodingEnvironment* eeCabacHf_, HighBandInfoContexts* highBandCtx_, bool qpDelta, bool hfTransformSkip);
        void hfMBOriReorder();
        void hfMBRecReorder(uint8_t bestTransType);
        int32_t DetermineMBdeltaQP(uint8_t subPicQP, uint32_t pos, uint32_t stride);
        void ChromaEnhanceQuant(FrameBuffer& mbPix);

    private:
        std::shared_ptr<EncoderQuant>encoderQuant_;
        std::shared_ptr<IQuant>hfIQuant_;
        std::shared_ptr<HFTransITrans>hfTransITrans_;
        std::shared_ptr<HFEncoderEntropy>hfEncoderEntropy_;

        SubBandMap* subPic_;
        SubBandMap* subPicRec_;
        uint8_t qp_[N_SUB_BANDS][N_COLOR_COMP];
        uint8_t mbQp_[N_SUB_BANDS][N_COLOR_COMP];
        uint32_t hfBandWidth_, hfBandHeight_;
        uint8_t colorComponent_;
        uint8_t componentShiftX_;
        uint8_t hfBandIdx_;
        uint32_t pixelIndex_;
        uint32_t lineWidth_;
        uint8_t bestTransType_;
        FrameBuffer mbOri_;
        FrameBuffer mbHad_;
        FrameBuffer mbCoeff_;
        FrameBuffer mbRec_;
    };
}

#endif // LL_ENCODER_H
