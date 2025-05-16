/* ====================================================================================================================

Copyright(c) 2025, HUAWEI TECHNOLOGIES CO., LTD.
Licensed under the Code Sharing Policy of the UHD World Association(the "Policy");
http://www.theuwa.com/UWA_Code_Sharing_Policy.pdf.
you may not use this file except in compliance with the Policy.
Unless agreed to in writing, software distributed under the Policy is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OF ANY KIND, either express or implied.
See the Policy for the specific language governing permissions and
limitations under the Policy.

* ====================================================================================================================*/
#ifndef BASIC_TYPES_H
#define BASIC_TYPES_H

namespace ProVivid {
    enum class PixelFormat {
        YUV420P10LE,
        YUV422P10LE,
        YUV444P10LE
    };

    enum ColorComp {
        Y = 0,
        U,
        V,
        N_COLOR_COMP
    };

    enum SubBandType {
        LL = 0,
        LH,
        HL,
        HH,
        N_SUB_BANDS
    };
}

#endif  // BASIC_TYPES_H