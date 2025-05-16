#ifndef BASIC_TYPES_H
#define BASIC_TYPES_H

#include <cstdint>
#include <string>
#include <unordered_map>

namespace ProVivid {
    using Pel = int32_t;
    using PelStorage = int16_t;

    enum class PixelFormat {
        YUV444P10LE,
        YUV422P10LE,
        YUV420P10LE
    };

    enum ColorComp {
        Y = 0,
        U,
        V,
        N_COLOR_COMP
    };
    static const ColorComp COLORS[] = {Y, U, V};
    static const std::string COLORS_STR[] = {"Y", "U", "V"};

    enum SubBandType {
        LL = 0,
        HL,
        LH,
        HH,
        N_SUB_BANDS
    };
    static const SubBandType SUB_BANDS[] = {LL, HL, LH, HH};
    static const std::string SUB_BANDS_STR[] = {"LL", "HL", "LH", "HH"};

    typedef struct ImgBufSize {
        uint32_t w;
        uint32_t h;
        uint32_t strideW;
        uint32_t strideH;
    } ImgBufSize;

    enum class SubPicBorder {
        NONE,
        RIGHT,
        BOTTOM,
        CORNER
    };

    typedef struct SubPicInfo {
        uint32_t id;
        uint32_t x;
        uint32_t y;
        uint32_t w;
        uint32_t h;
        uint32_t strideW;
        uint32_t strideH;
        SubPicBorder border;
        void* picHeaderPtr;
    } SubPicInfo;

    typedef struct MotionVector {
      int32_t mvX;
      int32_t mvY;
      MotionVector(): mvX(0), mvY(0) {}
      MotionVector(int32_t x, int32_t y): mvX(x), mvY(y) {}
    } MotionVector;

	enum {
		MB_NBR_UP = 0,
		MB_NBR_LEFT,
		N_NBR
	};

	enum {
		LUMA = 0,
		CHROMA,
		LUMA_CHROMA
	};

	enum PU_SIZE{
		LUMA_PU_4x4 = 0,
		CHROMA_PU_4x8 = LUMA_PU_4x4,
		LUMA_PU_8x8,
		N_TU_SIZE
	};

	enum PRED_MODE {
		INTRA_VER,
		INTRA_DC,
        INTER_PRED = INTRA_DC,
		INTRA_HOR,
		INTRA_PLANAR,
		INTRA_CCLM= INTRA_PLANAR,
		N_PRED_PRED
	};

	enum TRANS_TYPE {
		DCT2 = 0,
		DST7,
		N_TRANS
	};

    typedef enum FrameType {
        FRAME_I = 0,
        FRAME_P,
        FRAME_TYPE
    } FrameType;

    enum {
        MB_I = 0,
        MB_P,
        MB_TYPE
    };

    enum {
        HF_HAD = 0,
        HF_NOHAD,
    };

    using SubPicInfoMap = std::unordered_map<ColorComp, SubPicInfo>;
}

#endif  // BASIC_TYPES_H
