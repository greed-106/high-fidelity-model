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
#ifndef TABLE_H
#define TABLE_H

#include <istream>

namespace HFM {
    const int8_t DST7_W4[4][4] = {
        {15, 27, 37, 42},
        {37, 37, 0, -37},
        {42, -15, -37, 27},
        {27, -42, 37, -15}
    };


    const int8_t DCT2_W4[4][4] = {
        {32, 32, 32, 32},
        {42, 17, -17, -42},
        {32, -32, -32, 32},
        {17, -42,  42, -17}
    };

    const int8_t DCT2_W8[8][8] = {
        {32, 32, 32, 32, 32, 32, 32, 32},
        {44, 38, 25, 9, -9, -25, -38, -44 },
        {42, 17, -17, -42, -42, -17, 17, 42},
        {38, -9, -44, -25, 25, 44, 9, -38},
        {32, -32, -32, 32, 32, -32 - 32, 32},
        {25, -44, 9, 38, -38, -9, 44, -25},
        {17, -42, 42, -17, -17, 42, -42, 17},
        {9, -25, 38, -44, 44, -38, 25, -9}
    };

    const uint32_t QUANT_SCALE[3][8] = {
        {//4x4
            512, 468, 431, 395, 360, 331, 303, 280,
        },
        {//4x8
            728, 669, 607, 565, 512, 475, 431, 395,
        },
        {//8x8
            1023, 936, 862, 799, 728 , 669, 607, 555,
        },
    };

    const uint32_t DEQUANT_SCALE[3][8] = {
        {//4x4
            64, 70, 76, 83, 91, 99, 108, 117,
        },
        {//4x8
            45, 49, 54, 58, 64, 69, 76, 83,
        },
        {//8x8
            32, 35, 38, 41, 45, 49, 54, 59,
        },
    };

    const uint32_t HF_REMAP_LINE[64] = {
        0, 0, 1, 1, 0, 0, 1, 1,
        2, 2, 3, 3, 2, 2, 3, 3,
        4, 4, 5, 5, 4, 4, 5, 5,
        6, 6, 7, 7, 6, 6, 7, 7,
        0, 0, 1, 1, 0, 0, 1, 1,
        2, 2, 3, 3, 2, 2, 3, 3,
        4, 4, 5, 5, 4, 4, 5, 5,
        6, 6, 7, 7, 6, 6, 7, 7,
    };

    const uint32_t HF_REMAP_POS_16[16] = {
        0, 1, 0, 1, 2, 3, 2, 3,
        4, 5, 4, 5, 6, 7, 6, 7,
    };

    const float LAMBDA[40] = {
        10.000000f, 11.892071f, 14.142136f, 16.817928f,
        20.000000f, 23.784142f, 28.284271f, 33.635857f,
        40.000000f, 47.568283f, 56.568542f, 67.271713f,
        80.000000f, 95.136566f, 113.137085f, 134.543427f,
        160.000000f, 190.273132f, 226.274170f, 269.086853f,
        320.000000f, 380.546265f, 452.548340f, 538.173706f,
        640.000000f, 761.092529f, 905.096680f, 1076.347412f,
        1280.000000f, 1522.185059f, 1810.193359f, 2152.694824f,
        2560.000000f, 3044.370117f, 3620.386719f, 4305.389648f,
        5120.000000f, 6088.740234f, 7240.773438f, 8610.779297f
    };

    const int CCLM_SCALE[16] = {
        63, 63, 61, 57, 54, 51, 49, 47, 45, 43, 41, 39, 38, 37, 35, 34,
    };
}

#endif  // TABLE_H
