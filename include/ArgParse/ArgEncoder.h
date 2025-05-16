/* ====================================================================================================================

  The copyright in this software is being made available under the License included below.
  This software may be subject to other third party and contributor rights, including patent rights, and no such
  rights are granted under this license.

  Copyright (c) 2025, HUAWEI TECHNOLOGIES CO., LTD. All rights reserved.
  Copyright (c) 2025, PEKING UNIVERSITY. All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted only for
  the purpose of developing standards within Audio and Video Coding Standard Workgroup of China (AVS) and for testing and
  promoting such standards. The following conditions are required to be met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and
      the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and
      the following disclaimer in the documentation and/or other materials provided with the distribution.
    * The name of HUAWEI TECHNOLOGIES CO., LTD. may not be used to endorse or promote products derived from
      this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
  INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

* ====================================================================================================================
*/
#ifndef ARG_ENCODER_H
#define ARG_ENCODER_H

#include "ArgParse.h"
#include "Utils.h"
#include "Tool.h"
namespace HFM {
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
