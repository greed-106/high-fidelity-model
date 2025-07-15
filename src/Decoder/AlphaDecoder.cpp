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
#include "AlphaDecoder.h"


namespace HFM {
    int AlphaDecoder::ReadKGolo(int k) {
        int ws = 0;
        while (read_u_v(1, &bitstreamVlcAlpha_) == 0) {
            ws++;
            if (ws > 20) {
                exit(-1);
            }
        }
        int val = read_u_v(ws, &bitstreamVlcAlpha_);
        val += (1 << ws);
        val = ((val - 1) << k);
        int wb = read_u_v(k, &bitstreamVlcAlpha_);
        val = val + wb;
        return (val);
    }

    void AlphaDecoder::Set(int width, int height, SharedFrameBuffer alphaBuffer, uint32_t alpha16bitFlag, Bitstream* bitstream, SubpicSyntaxInfo* subpicSyntaxInfo) {
        picWidth_ = width;
        picHeight_ = height;
        alphaBuffer_ = alphaBuffer;
        alpha16bitFlag_ = alpha16bitFlag;
        bitstreamVlcAlpha_ = {0};
        bitstreamVlcAlpha_.streamBuffer = bitstream->streamBuffer + (bitstream->frame_bitoffset >> 3) + subpicSyntaxInfo->subpicLlCabacLength \
            + subpicSyntaxInfo->subpicLlVlcLength + subpicSyntaxInfo->subpicHfCabacLength + subpicSyntaxInfo->subpicHfVlcLength;
        bitstreamVlcAlpha_.bitstream_length = subpicSyntaxInfo->subpicLength- (subpicSyntaxInfo->subpicLlCabacLength 
            + subpicSyntaxInfo->subpicLlVlcLength + subpicSyntaxInfo->subpicHfCabacLength + subpicSyntaxInfo->subpicHfVlcLength);
    }

    int AlphaDecoder::ReadDiff() {
        int sign = read_u_v(1, &bitstreamVlcAlpha_);
        int diff = ReadKGolo(0) + 1;
        diff *= (sign ? 1 : -1);
        return diff;
    }

    int AlphaDecoder::ReadRunlength() {
        //3th golomb
        int runlength = ReadKGolo(3);
        return (runlength + 2);
    }

    void AlphaDecoder::AlphaDecode() {
        int elementNum = picWidth_ * 16;
        if (elementNum > 16384) elementNum = 16384;
        int groupNum = (picHeight_*picWidth_ + elementNum - 1) / elementNum;
        Pel* ptr = alphaBuffer_->data();
        int val;
        int preVal;
        int diff;
        int runLength;
        int RepeatFlag;
        int alpha_group_flag = 0;
        for (int idx = 0; idx < groupNum; idx++) {
            int curNumbers = 0;
            int curElementNum;
            if (idx == groupNum - 1) {
                curElementNum = picWidth_ * picHeight_ - elementNum * idx;
            } else {
                curElementNum = elementNum;
            }
            if (alpha16bitFlag_) {
                val = read_u_v(16, &bitstreamVlcAlpha_);
            } else {
                val = read_u_v(8, &bitstreamVlcAlpha_);
            }
            alpha_group_flag = read_u_v(1, &bitstreamVlcAlpha_);
            if (alpha_group_flag) {
                for (int i = 0; i < curElementNum; i++)
                    (*alphaBuffer_).push_back(val);
                continue;
            } else {
                while (curNumbers < curElementNum) {
                    RepeatFlag = read_u_v(1, &bitstreamVlcAlpha_);
                    if (curNumbers != 0) {
                        diff = ReadDiff();
                        if (alpha16bitFlag_) {
                            val = (preVal + diff + 65536) % 65536;
                        } else {
                            val = (preVal + diff + 256) %256;
                        }
                    }
                    if (RepeatFlag) {
                        runLength = ReadRunlength();
                        for (int i = 0; i < runLength; i++) {
                            (*alphaBuffer_).push_back(val);
                        }
                        curNumbers += runLength;
                    } else {
                        (*alphaBuffer_).push_back(val);
                        curNumbers++;
                    }
                    preVal = val;
                }
            }
        }
    }

}