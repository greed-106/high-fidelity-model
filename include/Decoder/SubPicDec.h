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
#ifndef SUB_PIC_DEC_H
#define SUB_PIC_DEC_H

#include <array>
#include <fstream>
#include <memory>
#include <unordered_map>
#include "SubPic.h"

namespace ProVivid {
    class SubPicDec : public SubPic{
    public:
        SubPicDec() = delete;
        SubPicDec(PixelFormat pixelFormat, uint32_t picWidth, uint32_t picHeight, uint32_t subPicWidth, uint32_t subPicHeight);
        ~SubPicDec() = default;
        void SetLLReference(SubPicInfoMap& subPicLLInfoRef);
        SubBandMap subBands_;
        SubBandMap subBandsRef_;
        SharedFrameBuffer dwtRowBuffer_;
        SharedFrameBuffer dwtTransTmpBuffer_;
    private:
        uint32_t subBandPixels_[N_COLOR_COMP]{};
    };
}

#endif // SUB_PIC_DEC_H
