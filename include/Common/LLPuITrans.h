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
#ifndef LL_PU_I_TRANS_H
#define LL_PU_I_TRANS_H

#include <vector>
#include "BasicTypes.h"
#include "Const.h"
#include "Utils.h"
#include "Table.h"
#include "Tool.h"

namespace ProVivid {

    class LLPuITrans {
    public:
        LLPuITrans();
        ~LLPuITrans();
        void Set(uint8_t puWidth, uint8_t puHeight, uint8_t predMode);
        void ComLLITrans(std::vector<int32_t>& coeff);
        void ITrans(std::vector<int32_t>& src, std::vector<int32_t>& dst, uint8_t is8, TRANS_TYPE transMode, uint8_t length, uint8_t shift);
        void DST7W4(std::vector<int32_t>& src, std::vector<int32_t>& dst, uint8_t length, uint8_t shift);
        void DCT2W4(std::vector<int32_t>& src, std::vector<int32_t>& dst, uint8_t length, uint8_t shift);
        void DCT2W8(std::vector<int32_t>& src, std::vector<int32_t>& dst, uint8_t length, uint8_t shift);
    private:
        uint8_t puWidth_, puHeight_;
        TRANS_TYPE horITransMode_, verITransMode_;
        std::vector<int32_t> coeffITransVer_;
    };
}

#endif // LL_PU_RECON_H
