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
#ifndef I_QUANT_H
#define I_QUANT_H

#include <vector>
#include "FrameBuffer.h"
#include "Table.h"
#include "Const.h"
#include "Utils.h"

namespace ProVivid {

    class IQuant {
    public:
        IQuant();
        ~IQuant();
        void Set(uint8_t puWidth, uint8_t  puHeight, uint8_t qp, uint32_t iQuantRangeBit);
        void ComIQuant(FrameBuffer& src, FrameBuffer& dst, uint8_t sizeIndex);
    private:
        uint8_t puWidth_,puHeight_;
        uint8_t qp_;
        int32_t maxIQuantVal_, minIQuantVal_;
    };
}

#endif // LL_PU_RECON_H
