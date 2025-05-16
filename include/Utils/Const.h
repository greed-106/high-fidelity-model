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
#ifndef CONST_H
#define CONST_H

#include <istream>

namespace ProVivid {
#define MB_SIZE (8)

#define DWT_SUB_PIC_OFFSET (1024)
#define INTRA_DEFAULT (1<<12)
#define LL_TRANS1_SHIFT (5)
#define LL_TRANS2_SHIFT (7)
#define LL_ITRANS1_SHIFT (5)
#define LL_ITRANS2_SHIFT (7)
#define LL_TRANS_DYNAMIC_BIT (15)
#define QUANT_SHIFT (11)
#define DEQUANT_SHIFT (4)
#define LL_QUANT_DYNAMIC_BIT (12)
#define LL_TH1               (300)
#define LL_DEADZONE          (320)
#define LL_RDOQ_OFFSET       (250)
#define LL_IQUANT_DYNAMIC_BIT (15)
#define LL_ITRANS1_DYNAMIC_BIT (15)
#define LL_ITRANS2_DYNAMIC_BIT (13)
#define LL_REC_DYNAMIC_BIT     (13)

#define HF_QUANT_DYNAMIC_BIT (8)
#define HF_TH1               (313)
#define HF_DEADZONE          (341)
#define HF_RDOQ_OFFSET       (200)
#define HF_IQUANT_DYNAMIC_BIT (13)
#define HF_ITRANS_DYNAMIC_BIT (12)

#define CCLM_SHIFT1 (13)
#define CCLM_SHIFT2 (10)
#define MAX_CCLM_DIFF_UV_BIT (12)
#define MAX_CCLM_PRED_BIT (13)

// inter prediction
#define MV_HOR_MAX 7
#define MV_VER_MAX 3
#define REF_HOR_SIZE  (MB_SIZE+2*MV_HOR_MAX)
#define REF_VER_SIZE  (MB_SIZE+2*MV_VER_MAX)
}

#endif  // CONST_H
