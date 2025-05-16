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
#include <fstream>
#include "ArgParse.h"
#include "BasicTypes.h"
#include "Decoder.h"
#include "Log.h"
#include "SubPicDec.h"
#include "Video.h"

extern "C" {
#include "vlc.h"
#include "Entropy.h"
}

namespace {
    const int EXIT_CODE_SUCCESS = 0;
    const int EXIT_CODE_ERROR = 1;
} // namespace

using namespace HFM;

struct Arguments: public argparse::Args {
    std::string& bitstream = kwarg("b,Bitstream", "input bitstream file");
    std::string& decFile = kwarg("o,DecFile", "output image file");
    std::string& decLLFile = kwarg("DecLLFile", "output LL sub-band image file").set_default("");
};

SeqPicHeaderInfo ParsePartialHeaderInfo(const std::string& bitstreamFile)
{
    const uint32_t REQ_HEADER_BYTES = 10;
    byte streamBuffer[REQ_HEADER_BYTES];
    Bitstream bitstream{0};
    bitstream.bitstream_length = REQ_HEADER_BYTES;
    bitstream.streamBuffer = streamBuffer;

    std::ifstream headerStream;
    headerStream.open(bitstreamFile, std::ifstream::binary);
    if (headerStream.fail()) {
        LOGE("failed to open: %s\n", bitstreamFile.c_str());
    }
    headerStream.read(reinterpret_cast<char*>(streamBuffer), REQ_HEADER_BYTES * sizeof(byte));
    SeqPicHeaderInfo seqPicHeaderInfo{};
    seqPicHeaderInfo.profileIdc = read_u_v(8, &bitstream);
    seqPicHeaderInfo.levelIdc = read_u_v(8, &bitstream);
    seqPicHeaderInfo.frameCount = read_u_v(2, &bitstream) + 1;
    seqPicHeaderInfo.frameRate = read_u_v(8, &bitstream);
    seqPicHeaderInfo.width = read_u_v(16, &bitstream);
    seqPicHeaderInfo.height = read_u_v(16, &bitstream);
    seqPicHeaderInfo.subPicWidth = (read_u_v(3, &bitstream) + 2) << 7;
    seqPicHeaderInfo.subPicHeight = (read_u_v(6, &bitstream) + 2) << 7;
    seqPicHeaderInfo.bitDepth = read_u_v(4, &bitstream) + 8;
    seqPicHeaderInfo.pixelFormat = read_u_v(2, &bitstream); // 0: YUV444, 1: YUV422, 2: YUV420
    if (headerStream.is_open()) {
        headerStream.close();
    }
    return seqPicHeaderInfo;
}

int main(int argc, const char** argv)
{
    Timer timer("DecoderApp::" + std::string(__FUNCTION__));
    auto args = [argc, &argv] () -> Arguments {
        try {
            return argparse::parse<Arguments>(argc, argv, /*raise_on_error*/ true);
        } catch (const std::runtime_error &error) {
            LOGE("failed to parse arguments: %s\n", error.what());
            exit(EXIT_CODE_ERROR);
        }
    } ();
    // TODO: parse seq-pic-header info with a more efficient way
    auto headInfo = ParsePartialHeaderInfo(args.bitstream);
    auto pixelFormat = static_cast<PixelFormat>(headInfo.pixelFormat);
    auto subPic = std::make_shared<SubPicDec>(pixelFormat, headInfo.width, headInfo.height,
                                              headInfo.subPicWidth, headInfo.subPicHeight);
    auto decoder = std::make_shared<Decoder>(headInfo.bitDepth, args.decFile, args.decLLFile, args.bitstream, headInfo.frameCount);
    decoder->SetSubPic(subPic);
    for (int frameIdx = 0; decoder->curBitstreamPos_ < decoder->bitstreamLength_; ++frameIdx) {
        LOGI("decode frame %d\n", frameIdx);
        std::string frameDesc{"frame: " + std::to_string(frameIdx)};
        timer.Start(frameDesc);
        decoder->ParseSeqPicHeaderInfo(frameIdx, &decoder->bitstream_);
        decoder->Decode(frameIdx);
        timer.End(frameDesc);
    }
    return EXIT_CODE_SUCCESS;
}
