#ifndef LL_PU_I_TRANS_H
#define LL_PU_I_TRANS_H

#include <vector>
#include "BasicTypes.h"
#include "Const.h"
#include "Utils.h"
#include "Table.h"

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
