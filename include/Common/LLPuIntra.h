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
#ifndef LL_PU_INTRA_H
#define LL_PU_INTRA_H

#include <fstream>
#include <memory>
#include <unordered_map>
#include "FrameBuffer.h"
#include "BasicTypes.h"
#include "Const.h"

namespace ProVivid {

    class LLPuIntra {
    public:
        LLPuIntra();
        ~LLPuIntra();
        void GetPuNbr(SharedFrameBuffer llBandPicRec);
        void ComLLPred(std::vector<int32_t>&);
        void SetDefault(std::vector<int32_t>&);
        void IntraPredVer(std::vector<int32_t>&);
        void IntraPredDC(std::vector<int32_t>&);
        void IntraPredHor(std::vector<int32_t>&);
        void IntraPredPlanar(std::vector<int32_t>&);
        void Set(uint8_t tuSize, uint8_t intraPredMode, uint32_t isChroma,
            uint8_t puX, uint8_t puY, uint8_t puWidth, uint8_t puHeight, uint32_t mbX, uint32_t mbY
            , uint32_t puPixelIndex, uint32_t lineWidth);
    private:
        SharedPixelBuffer Nbr_[N_NBR];
        uint32_t puSize_;
        uint32_t intraPredMode_;
        uint32_t isChroma_;
        uint32_t puWidth_, puHeight_;
        bool leftAvail_, upAvail_;
        uint32_t puPixelIndex_;
        uint32_t lineWidth_;
    };
}

#endif // LL_PU_INTRA_H
