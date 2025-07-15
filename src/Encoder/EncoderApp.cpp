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
#include <memory>
#include <utility>
#include "ArgEncoder.h"
#include "BasicTypes.h"
#include "CfgParser.h"
#include "Encoder.h"
#include "Log.h"
#include "Video.h"
#include "SubPicEnc.h"
#include "Status.h"

extern "C" {
#include "vlc.h"
#include "Entropy.h"
}

namespace {
    const int EXIT_CODE_SUCCESS = 0;
    const int EXIT_CODE_ERROR = 1;
} // namespace

using namespace HFM;

SeqPicHeaderInfo ParsePicHeader(const Arguments& args)
{
    return {args.profileIdc, args.levelIdc,
            args.intraPeriod, args.frameRate,
            args.width, args.height,
            args.subPicWidth, args.subPicHeight,
            args.bitDepth,
            static_cast<uint32_t>(args.pixelFormat),
            args.inputAlphaFlag,
            args.inputAlpha16bitFlag,
            0,//alphaMapCodeMode
            args.qpDeltaEnable,
            args.hfTransformSkip,
            args.cclmEnable};
}

int main(int argc, const char** argv) {
    Timer timer("EncoderApp::" + std::string(__FUNCTION__));
    auto args = [argc, &argv] () -> Arguments {
        try {
            return argparse::parse<Arguments>(argc, argv, /*raise_on_error*/ true);
        } catch (const std::runtime_error &error) {
            LOGE("failed to parse arguments: %s\n", error.what());
            exit(EXIT_CODE_ERROR);
        }
    } ();
    auto cfgParser = std::make_shared<CfgParser>(argc, argv);
    cfgParser->UpdateArgs(args);
    LOGI("encoder parameters:\n %s", args.print().c_str());

    auto pixelFormat = static_cast<PixelFormat>(args.pixelFormat);
    AlphaInput alphaInput = {args.inputAlphaFlag, args.inputAlpha16bitFlag, args.inputAlphaFile};
    QPGroup qpGroup = {args.qp, args.pFrameQpOffset, args.cbQpOffset, args.crQpOffset,
        args.hlQpOffset, args.lhQpOffset, args.hhQpOffset};
    auto encoder = std::make_shared<Encoder>(args.recFile, args.recLLFile,
                                             args.frameCount, args.bitDepth, args.intraPeriod);
    auto video = std::make_shared<Video>(pixelFormat, args.frameCount, args.width, args.height, args.inputFile, alphaInput);
    auto subPic = std::make_shared<SubPicEnc>(pixelFormat, args.width, args.height, args.subPicWidth, args.subPicHeight);
    subPic->UpdateInfo(video->GetCurrFrame());
    if (!args.dwtSubPicPath.empty()) {
        subPic->SetDWTSubPicPath(args.dwtSubPicPath);
    }
    Bitstream* bitstream = &encoder->bitstream_;
    auto seqPicHeaderInfo = ParsePicHeader(args);
    for (int i = 0; i < args.frameCount; ++i) {
        LOGI("encode frame %d\n", i);
        std::string frameDesc{"frame: " + std::to_string(i)};
        BitstreamInit(bitstream, 32);
        encoder->seqHeaderBytes_ = WriteSeqPicHeader(i, args.intraPeriod, &seqPicHeaderInfo, bitstream);
        subPic->GetFrame(video->MoveToNextFrame());
        timer.Start(frameDesc);
        encoder->SetInput(pixelFormat, subPic, alphaInput, qpGroup,
            args.qpDeltaEnable, args.hfTransformSkip, args.cclmEnable);
        encoder->Encode(i);
        BitstreamWrite(i, bitstream, (char*) args.bitstream.data());
        timer.End(frameDesc);
    }
    return EXIT_CODE_SUCCESS;
}
