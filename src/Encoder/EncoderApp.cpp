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

using namespace ProVivid;

SeqPicHeaderInfo ParsePicHeader(const Arguments& args)
{
    return {args.profileIdc, args.levelIdc,
            args.intraPeriod, args.frameRate,
            args.width, args.height,
            args.subPicWidth, args.subPicHeight,
            args.bitDepth,
            static_cast<uint32_t>(args.pixelFormat),
            args.qpDeltaEnable,
            args.hfTransformSkip};
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
    auto encoder = std::make_shared<Encoder>(args.recFile, args.recLLFile,
                                             args.frameCount, args.bitDepth, args.intraPeriod);
    auto video = std::make_shared<Video>(pixelFormat, args.frameCount, args.width, args.height, args.inputFile);
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
        BitstreamInit(bitstream, 29);
        encoder->seqHeaderBytes_ = WriteSeqPicHeader(i, args.intraPeriod, &seqPicHeaderInfo, bitstream);
        subPic->GetFrame(video->MoveToNextFrame());
        timer.Start(frameDesc);
        encoder->SetInput(subPic, args.qp, args.cbQpOffset, args.crQpOffset, args.hlQpOffset, args.lhQpOffset, args.hhQpOffset,
            args.qpDeltaEnable, args.hfTransformSkip);
        encoder->Encode(i);
        BitstreamWrite(i, bitstream, (char*) args.bitstream.data());
        timer.End(frameDesc);
    }
    return EXIT_CODE_SUCCESS;
}
