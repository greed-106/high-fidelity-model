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
    uint32_t qpDeltaEnable;
    uint32_t hfTransformSkip;
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
