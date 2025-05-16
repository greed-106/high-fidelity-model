/* ====================================================================================================================

  The copyright in this software is being made available under the License included below.
  This software may be subject to other third party and contributor rights, including patent rights, and no such
  rights are granted under this license.

  Copyright (c) 2025, HUAWEI TECHNOLOGIES CO., LTD. All rights reserved.
  Copyright (c) 2025, PEKING UNIVERSITY. All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted only for
  the purpose of developing standards within Audio and Video Coding Standard Workgroup of China (AVS) and for testing and
  promoting such standards. The following conditions are required to be met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and
      the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and
      the following disclaimer in the documentation and/or other materials provided with the distribution.
    * The name of HUAWEI TECHNOLOGIES CO., LTD. may not be used to endorse or promote products derived from
      this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
  INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

* ====================================================================================================================
*/
#ifndef CONST_H
#define CONST_H

#include <istream>

namespace HFM {
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
