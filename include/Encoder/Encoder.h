#ifndef ENCODER_H
#define ENCODER_H

#include <array>
#include <vector>
#include "SubPicEnc.h"
extern "C" {
#include "vlc.h" 
#include "cabac.h"
#include "biariencode.h"
#include "context_ini.h"
}
#include "HFEncoder.h"
#include "LLEncoder.h"
#include "SubPic.h"
#include "Timer.h"

namespace ProVivid {
    class Encoder {
    public:
        Encoder() = delete;
        Encoder(std::string recFile, std::string recLLFile, uint32_t frameCount, uint32_t bitDepth, uint32_t intraPeriod);
        ~Encoder();
        void SetInput(std::shared_ptr<SubPicEnc> subPic, 
            int32_t qp, int32_t cbQpOffset, int32_t crQpOffset, int32_t hlQpOffset, int32_t lhQpOffset, int32_t hhQpOffset,
            bool qpDeltaEnable, bool hfTransformSkip);
        void Encode(uint32_t currFrame);
        void StreamCabacCtxIni();
        void SubpicEncodingDone(uint32_t subpicIndex);
        void PicEncodingDone();
        Bitstream bitstream_;
        int seqHeaderBytes_;
    private:
        std::string recFile_;
        std::string recLLFile_;
        uint32_t frameCount_;
        uint32_t bitDepth_;
        uint32_t intraPeriod_;
        uint8_t qp_;
        int32_t cbQpOffset_, crQpOffset_, hlQpOffset_, lhQpOffset_, hhQpOffset_;
        bool qpDeltaEnable_, hfTransformSkip_;
        std::ofstream recFileHandle_;
        std::ofstream recLLFileHandle_;
        std::shared_ptr<SubPicEnc> subPic_;
        std::vector<SubPicInfoMap> subPicInfoRec_;
        std::vector<SubPicInfoMap> subPicLLInfoRec_;
        std::vector<SubPicInfoMap> subPicLLInfoRef_;
        SharedBufferStorage recPicBuffer_;
        SharedBufferStorage recPicLLBuffer_;
        std::vector<std::pair<uint32_t, uint32_t>> picWHRaw_{};
        std::vector<std::pair<uint32_t, uint32_t>> picWHRawLL_{};
        std::array<ImgBufSize, LUMA_CHROMA> recPicSize_{};
        SharedBufferStorage refPicLLBuffer_;
        Bitstream bitstreamCabacLl_;
        Bitstream bitstreamVlcLl_;
        Bitstream bitstreamCabacHf_;
        Bitstream bitstreamVlcHf_;
        EncodingEnvironment eeCabacLl_;
        EncodingEnvironment eeCabacHf_;
        TextureInfoContexts texCtx_;
        MotionInfoContexts  motCtx_;
        HighBandInfoContexts highBandCtx_;

    };
}

#endif // ENCODER_H
