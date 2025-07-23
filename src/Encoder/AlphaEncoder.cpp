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
#include "AlphaEncoder.h"
//#include <iostream>

namespace HFM {
    void AlphaEncoder::WriteKGolo(int k, int val) {
        int wb = val & ((1 << k) - 1);
        val >>= k;
        val += 1;
        int ws = floor(log2(val));

        write_u_v(ws, 0, bitstreamVlcAlpha_);
        write_u_v(ws + 1, val, bitstreamVlcAlpha_);
        write_u_v(k, wb, bitstreamVlcAlpha_);
    }

    void AlphaEncoder::Write0Golo(int val) {
        int ws = floor(log2(val));
        write_u_v(ws, 0, bitstreamVlcAlpha_);
        write_u_v(ws + 1, val, bitstreamVlcAlpha_);
    }

    void AlphaEncoder::Set(int width, int height, SharedBufferStorage alphaBuffer, uint32_t inputAlpha16bitFlag, Bitstream* bitstreamVlcAlpha) {
        picWidth_ = width;
        picHeight_ = height;
        alphaBuffer_ = alphaBuffer;
        inputAlpha16bitFlag_ = inputAlpha16bitFlag;
        bitstreamVlcAlpha_ = bitstreamVlcAlpha;
    }

    void AlphaEncoder::WriteDiff(int diff) {
        if (inputAlpha16bitFlag_) {
            if (diff > 32767) diff -= 65536;
            if (diff < -32768) diff += 65536;
        } else {
            if (diff > 127) diff -= 256;
            if (diff < -128) diff += 256;
        }

        write_u_v(1, diff > 0, bitstreamVlcAlpha_);
        diff = abs(diff);
        if (diff == 0) {
            printf("error");
            exit(-1);
        }
        //0th golomb
        Write0Golo(diff);
    }

    void AlphaEncoder::WriteRunlength(int runlength) {
        if (runlength < 2) {
            printf("error");
            exit(-1);
        }
        //3th golomb
        WriteKGolo(3, runlength - 2);
    }

    void AlphaEncoder::AlphaEncode() {
        int elementNum = picWidth_ * 16;
        if (elementNum > 16384) elementNum = 16384;
        int groupNum = (picHeight_*picWidth_ + elementNum - 1) / elementNum;
        PelStorage* ptr = alphaBuffer_->data();
        for (int idx = 0; idx < groupNum; idx++) {
            int curElementNum;
            if (idx == groupNum - 1) {
                curElementNum = picWidth_ * picHeight_ - elementNum * idx;
            } else {
                curElementNum = elementNum;
            }
            int curPix = 0;
            int prePix = -1;
            PelStorage preVal = 0;
            while (curPix < curElementNum) {
                while (curPix < curElementNum - 1 && ptr[curPix] == ptr[curPix + 1]) {
                    curPix++;
                }
                if (prePix == -1) {
                    if (inputAlpha16bitFlag_) {
                        write_u_v(16, ptr[curPix], bitstreamVlcAlpha_);
                    } else {
                        write_u_v(8, ptr[curPix], bitstreamVlcAlpha_);
                    }
                    write_u_v(1, curPix == curElementNum - 1, bitstreamVlcAlpha_);
                    if (curPix == curElementNum - 1) break;
                }
                write_u_v(1, curPix > prePix + 1, bitstreamVlcAlpha_);
                if (prePix != -1)
                    WriteDiff(ptr[curPix] - preVal);
                if (curPix > prePix + 1)
                    WriteRunlength(curPix - prePix);
                preVal = ptr[curPix];
                prePix = curPix;
                curPix++;
            }
            ptr += curElementNum;
        }
    }
}