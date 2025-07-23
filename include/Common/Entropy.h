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
#ifndef ENTROPY_H
#define ENTROPY_H

#include <stdint.h>
#include <stdbool.h>

typedef struct SeqPicHeader SeqPicHeaderInfo;

struct SeqPicHeader
{
    uint32_t profileIdc; 
    uint32_t levelIdc;
    uint32_t frameCount;
    uint32_t frameRate;
    uint32_t width;
    uint32_t height;
    uint32_t subPicWidth;
    uint32_t subPicHeight;
    uint32_t bitDepth;
    uint32_t pixelFormat;
    uint32_t alphaFlag;
    uint32_t alpha16bitFlag;
    uint32_t alphaMapCodeMode;
    uint32_t qpDeltaEnable;
    uint32_t hfTransformSkip;
    uint32_t cclmEnable;
    uint32_t picSize;
    uint32_t frameType;
    int numOfSubpic;
};

typedef struct SubpicSyntax SubpicSyntaxInfo;

struct SubpicSyntax
{
    uint32_t subpicLlQpIndex;
    int32_t subpicHlQpIndexOffset;
    int32_t subpicLhQpIndexOffset;
    int32_t subpicHhQpIndexOffset;
    int32_t subpicCbQpIndexOffset;
    int32_t subpicCrQpIndexOffset;
    uint32_t subpicLength;
    uint32_t subpicLlCabacLength;
    uint32_t subpicLlVlcLength;
    uint32_t subpicHfCabacLength;
    uint32_t subpicHfVlcLength;

    uint32_t subpicWidth;
    uint32_t subpicHeight;
};

typedef struct MbSyntax MbSyntaxInfo;

struct MbSyntax
{
    int32_t qpDelta;
    bool mbMode;
    bool interNoResidualFlag;
    bool interMvdFlag;
    int32_t puMvdX;
    int32_t puMvdY;
    int32_t leftPuMvdX;
    int32_t leftPuMvdY;
    uint32_t tuSizeLuma;
    uint32_t PredmodeLuma;
    uint32_t PredmodeChroma;
};

static inline int imin(int a, int b)
{
    return ((a) < (b)) ? (a) : (b);
}
static inline int imax(int a, int b)
{
    return ((a) > (b)) ? (a) : (b);
}

enum PreditionMode {
    INTRA_VER = 0,
    INTRA_DC,
    INTRA_HOR,
    OTHER,   //if intra luma, planar; if intra chroma, cclm 
    PRE_MODE_FOR_SCAN
};

static const int PermuteCoeffsTable[3][16] = {
    {
        2, 6, 10, 14, 0, 4, 8, 12,
        3, 7, 11, 15, 1, 5, 9, 13,
    },
    {
        1, 5, 9, 13, 0, 4, 8, 12,
        3, 7, 11, 15, 2, 6, 10, 14,
    },
    {
        0, 4, 8, 12, 1, 5, 9, 13,
        3, 7, 11, 15, 2, 6, 10, 14,
    }
};


#endif // ENTROPY_H
