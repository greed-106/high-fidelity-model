#ifndef LL_DECODER_ENTROPY_H
#define LL_DECODER_ENTROPY_H

#include <cstring>
#include "FrameBuffer.h"
extern "C" {
#include "cabac.h"
#include "vlc.h"
#include "biaridecod.h"
}

#include <vector>

namespace ProVivid {	
	class LLDecoderEntropy {
	public:
		LLDecoderEntropy(Bitstream* bitstream, SubpicSyntaxInfo* subpicSyntaxInfo);
		~LLDecoderEntropy();
        void LLEntropyCoeff(int component, std::vector<int32_t>& residual);
        void LLEntropyMbInfo(uint32_t frameType, uint32_t qpDeltaEnable);
        MbSyntaxInfo mbEntropyInfo_;

	private:
        Bitstream bitstreamCabacLl_;
        Bitstream bitstreamVlcLl_;
        DecodingEnvironment de_;
        TextureInfoContexts texCtx_;
        MotionInfoContexts motCtx_;
	};
}

#endif // LL_DECODER_ENTROPY_H
