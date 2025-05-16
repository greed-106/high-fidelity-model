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
#include "IQuant.h"

namespace ProVivid {
    IQuant::IQuant() {
    }
    
    IQuant::~IQuant() {
    }

    void IQuant::Set(uint8_t puWidth, uint8_t  puHeight, uint8_t qp, uint32_t iQuantRangeBit) {
        puWidth_ = puWidth;
        puHeight_ = puHeight;
        qp_ = qp;
        maxIQuantVal_ = (1 << iQuantRangeBit) - 1;
        minIQuantVal_ = -(1 << iQuantRangeBit);
    }

    void IQuant::ComIQuant(FrameBuffer& src, FrameBuffer& dst, uint8_t sizeIndex) {
        uint32_t scale = DEQUANT_SCALE[sizeIndex][(qp_ + 12) % 8];
        int8_t iQShift = DEQUANT_SHIFT - (qp_ + 12) / 8;
        int32_t offset = (iQShift <= 0) ? 0 : (1 << (iQShift - 1));

        auto ptrSrc = &src[0];
        auto ptrDst = &dst[0];
        for (uint8_t i = 0; i < puHeight_; i++) {
            for (uint8_t j = 0; j < puWidth_; j++) {
                int32_t coeffTmp = *ptrSrc;
                if (iQShift < 0) {
                    coeffTmp = (coeffTmp*(int32_t)scale) << (-iQShift);
                } else {
                    coeffTmp = (coeffTmp*(int32_t)scale+offset) >> (iQShift);
                }
                Clip<int32_t, int32_t>(coeffTmp, coeffTmp, minIQuantVal_, maxIQuantVal_);
                (*ptrDst) = coeffTmp;
                ptrSrc++;
                ptrDst++;
            }
        }
    }
}

