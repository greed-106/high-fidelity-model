#ifndef LL_ENCODER_ENTROPY_H
#define LL_ENCODER_ENTROPY_H
extern "C" {
#include <cstring>
#include "cabac.h"
#include "vlc.h"
}

#include <vector>

namespace ProVivid {
    class LLEncoderEntropy {
    public:
        LLEncoderEntropy();
        ~LLEncoderEntropy();
        void Set(Bitstream* bitstreamVlcLl, EncodingEnvironment* eeCabacLl, TextureInfoContexts* texCtx, MotionInfoContexts* motCtx);
        void SetInter(uint8_t colorComponent);
        void LLEntropyCoeff(uint8_t colorComponent, uint8_t tuSize, uint8_t predMode, std::vector<int32_t>& residual);
        void LLEntropyPuSize(uint8_t isIntra, uint8_t colorComponent, uint8_t puSize);
        void LLEntropyIntraPredMode(uint8_t colorComponent, uint8_t predModeize);
        void LLEntropyMbMode(uint8_t isPframe, uint8_t mbMode);
        void LLEntropyInterMode(uint8_t interNoResidualFlag);
        void LLEntropyMvdMode(uint8_t mvdFlag);
        void LLEntropyMvd(int mvd, int mvp, int isMvdY, int cond);
        void LLEntropyDeltaQp(int isLeftBoundaryMb, int subpicLumaQp, int &mbQp, int &qp_delta);
        void GetCabcaState();
        void ResetCabcaState();
        uint32_t bits_;
    private:
        Bitstream* bitstream_;
        EncodingEnvironmentPtr eep_;
        TextureInfoContexts* texCtx_;
        MotionInfoContexts* motCtx_;

        EncodingEnvironment eeState_;
        int cabacLenState;
        TextureInfoContexts texCtxState_;
        MotionInfoContexts motCtxState_;

        int refQp_;
    };
}

#endif // LL_ENCODER_ENTROPY_H
