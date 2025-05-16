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
#ifndef HF_TRANS_I_TRANS_H
#define HF_TRANS_I_TRANS_H

#include <vector>
#include "FrameBuffer.h"
#include "Const.h"
#include "Utils.h"

namespace ProVivid {

    class HFTransITrans {
    public:
        HFTransITrans();
        ~HFTransITrans();
        void Set(uint8_t colorComponent, uint8_t isITrans);
        void ComHFTransITrans(FrameBuffer& src, FrameBuffer& dst);
    private:
        uint32_t pixNum_;
        uint8_t isITrans_;
    };
}

#endif // LL_PU_RECON_H
