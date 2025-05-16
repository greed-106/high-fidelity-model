#ifndef LL_PU_INTER_H
#define LL_PU_INTER_H

#include <fstream>
#include <memory>
#include <unordered_map>
#include "FrameBuffer.h"
#include "BasicTypes.h"
#include "Const.h"

namespace ProVivid {
	
	class LLPuInter {
	public:
		LLPuInter();
		~LLPuInter();
		void GeRefWindow(SharedFrameBuffer llBandPicRec_, uint32_t llBandWidth_, uint32_t compId, uint32_t mbY, uint32_t mbX, bool rowFirstFlag, bool rowLastFlag, bool colFirstFlag, bool colLastFlag);
		void GetRefPixel(Pel* src, Pel* dst, uint32_t srcStride, uint32_t dstStride, uint32_t height, uint32_t width);
		void InterPred(MotionVector puMv, std::vector<int32_t>& interPred, uint32_t compId);
		void InterPredInt(std::vector<int32_t>& pred);
		void InterPredHor(std::vector<int32_t>& pred);
		void InterPredVer(std::vector<int32_t>& pred);
		void InterPredHorVer(std::vector<int32_t>& pred);
		void Set(uint32_t compId, uint32_t puWidth, uint32_t puHeight);
	protected:
    SharedPixelBuffer refWinYUV_[N_COLOR_COMP];
    SharedPixelBuffer refWin_;
		int32_t puWidth_; 
    int32_t puHeight_;
    int32_t refWinWidth_;
    int32_t refWinHeight_;
    int32_t refWinStride_;
    int32_t refWinOrgIdx_;
    int32_t refPadWidth_;
    int32_t refPadHeight_;
    SharedPixelBuffer pred_;
	};
}

#endif // LL_PU_INTER_H
