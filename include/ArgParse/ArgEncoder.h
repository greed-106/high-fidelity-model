/* ====================================================================================================================

Copyright(c) 2025, HUAWEI TECHNOLOGIES CO., LTD.
Licensed under the Code Sharing Policy of the UHD World Association(the "Policy");
http://www.theuwa.com/UWA_Code_Sharing_Policy.pdf.
you may not use this file except in compliance with the Policy.
Unless agreed to in writing, software distributed under the Policy is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OF ANY KIND, either express or implied.
See the Policy for the specific language governing permissions and
limitations under the Policy.

* ====================================================================================================================
*/
#ifndef ARG_ENCODER_H
#define ARG_ENCODER_H

#include "ArgParse.h"
#include "Utils.h"
#include "Tool.h"
namespace ProVivid {
#define REGIST_KWARG(type, name, options, description, value) \
    type& name = kwarg_ext(options, description).set_default(value);

    struct Arguments: public argparse::Args {
        std::vector<std::string> argNames {};

        argparse::Entry& kwarg_ext(const std::string& key, const std::string& help, const std::optional<std::string>& implicit_value=std::nullopt) {
            argNames.emplace_back(Trim(SplitString(key, ",").back()));
            return kwarg(key, help, implicit_value);
        }

        REGIST_KWARG(std::string, cfgFile, "c", "config file", "");
        REGIST_KWARG(std::string, inputFile, "i,InputFile", "input video file", 0);
        REGIST_KWARG(std::string, bitstream, "b,BitstreamFile", "output bitstream file", "");
        REGIST_KWARG(std::string, recFile, "o,RecFile", "reconstructed image file", "");
        REGIST_KWARG(std::string, recLLFile, "RecLLFile", "reconstructed LL sub-band image file", "");
        REGIST_KWARG(std::string, dwtSubPicPath, "DWTSubPicPath", "path to write DWT sub-picture file", "");
        REGIST_KWARG(uint32_t, width, "w,Width", "with of input video", 0);
        REGIST_KWARG(uint32_t, height, "h,Height", "height of input video", 0);
        REGIST_KWARG(uint32_t, subPicWidth, "SubPicWidth", "sub-picture width", 512);
        REGIST_KWARG(uint32_t, subPicHeight, "SubPicHeight", "sub-picture height", 4352);
        REGIST_KWARG(uint32_t, frameCount, "f,FrameCount", "number of frames", 1)
        REGIST_KWARG(uint32_t, bitDepth, "BitDepth", "input data bit-depth", 10);
        REGIST_KWARG(int32_t, pixelFormat, "PixelFormat", "pixel format, 0:yuv444p10le, 1:yuv422p10le2,:yuv420p10le", 1);
        REGIST_KWARG(int32_t, qp, "q,QP", "initial qp value", 15);
        REGIST_KWARG(int32_t, cbQpOffset, "CbQpOffset", "cb qp offset", 2);
        REGIST_KWARG(int32_t, crQpOffset, "CrQpOffset", "cr qp offset", 2); 
        REGIST_KWARG(int32_t, hlQpOffset, "HlQpOffset", "hl qp offset", -2);
        REGIST_KWARG(int32_t, lhQpOffset, "LhQpOffset", "lh qp offset", -4);
        REGIST_KWARG(int32_t, hhQpOffset, "HhQpOffset", "hh qp offset", -6);
        REGIST_KWARG(uint32_t, intraPeriod, "IntraPeriod", "intra period number, support 1~4", 1);
        REGIST_KWARG(uint32_t, qpDeltaEnable, "QPDeltaEnable", "flag to enable AQ", 0);
#if TRANSKIP
        REGIST_KWARG(uint32_t, hfTransformSkip, "HFTransformSkip", "flag to enable HF transform skip", 1);
#else
        REGIST_KWARG(uint32_t, hfTransformSkip, "HFTransformSkip", "flag to enable HF transform skip", 0);
#endif
        REGIST_KWARG(uint32_t, profileIdc, "ProfileIdc", "profile idc", 1);
        REGIST_KWARG(uint32_t, levelIdc, "LevelIdc", "level idc", 71);
        REGIST_KWARG(uint32_t, frameRate, "FrameRate", "frame rate", 30);
        REGIST_KWARG(uint32_t, sourceType, "SourceType", "source type, 0:SDR, 1:PQ, 2:HLG", 0);
        REGIST_KWARG(uint32_t, qualityPreset, "QualityPreset", "quality preset, 0:ULTRA, 1:HIGH", 0);
        REGIST_KWARG(uint32_t, speedPreset, "SpeedPreset", "speed preset, 0:SLOW, 1:FAST", 0);
    };
}

#endif // ARG_ENCODER_H
