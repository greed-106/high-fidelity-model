#ifndef HF_ENCODER_ENTROPY_H
#define HF_ENCODER_ENTROPY_H
extern "C" {
#include "cabac.h"
#include "vlc.h"
#include "biariencode.h"
}

#include <vector>

namespace ProVivid {
    extern int VLCTable0(int value);

    typedef struct {
        uint8_t bandIdx;
        uint8_t colorComponent;
        int* leftCoefMax;
        uint8_t transformType;
    } EncDecBlockParams;

    typedef struct {
        uint8_t bandIdx;
        uint8_t colorComponent;
        int leftCoefMax[9];
        int curBlockSize;
        uint8_t transformType;
    } EncDecBlockGroupParams;

    class HFEncoderEntropy {
    public:
        HFEncoderEntropy();
        ~HFEncoderEntropy();
        void Set(Bitstream* bitstreamVlcHf, EncodingEnvironment* eeCabacHf, HighBandInfoContexts* highBandCtx);
        void HFEntropyCoeffGroupSet(int isLeftBoundaryMb, uint8_t bandIdx, uint8_t colorComponent, uint8_t transformType);
        void HFEntropyCoeffGroup(std::vector<int32_t>& residual, EncDecBlockGroupParams* bgParams, int hfTransformSkipEnable);
        void HFEntropyCoeffBlock(int blockIdx, int* residual, EncDecBlockParams* blockParams);
        void HFEntropyDeltaQp(int isLeftBoundaryMb, int subpicLumaQp, int &mbQp, int &mb_deltaQP);
        void HFEntropyFlag(BiContextTypePtr pBinCtx, int value);
        inline void WriteVLCTable0(int value);
        inline void WriteVLCTable1(int value);
        inline void WriteVLCTable2(int value);
        inline void WriteVLCTable3(int value);
        void HFEncPattern0001(int* resSubBlock);
        EncDecBlockGroupParams bgParams_;

    private:
        Bitstream *bitstream_;
        EncodingEnvironmentPtr eep_;
        HighBandInfoContexts* highBandCtx_;

        int refQp_;
        //uint32_t bits_;

    };
}

#endif // HF_ENCODER_ENTROPY_H
