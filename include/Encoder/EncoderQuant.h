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
#ifndef ENCODER_QUANT_H
#define ENCODER_QUANT_H

#include <fstream>
#include <vector>
#include "FrameBuffer.h"
#include "Const.h"
#include "Utils.h"
#include "Table.h"

namespace ProVivid {
    class EncoderQuant {
    public:
        EncoderQuant();
        ~EncoderQuant();
        void Set(uint8_t puWidth, uint8_t  puHeight,uint8_t qp, uint32_t deadzone, uint32_t threshold1, uint32_t rdoqOffset, uint8_t quantRangeBit);
        void Quant(FrameBuffer& src, FrameBuffer& dst, uint8_t sizeIndex);
    private:
        uint8_t qp_;
        uint8_t puWidth_, puHeight_;
        uint32_t deadzone_, threshold1_, rdoqOffset_;
        int32_t maxQuantVal_, minQuantVal_;
    };
}

#endif // ENCODER_QUANT_H
