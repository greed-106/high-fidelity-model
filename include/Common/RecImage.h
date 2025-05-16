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
#ifndef REC_IMAGE_H
#define REC_IMAGE_H

#include <string>
#include "FrameBuffer.h"
#include "Status.h"

namespace ProVivid {
    std::pair<uint32_t, uint32_t> GetRecSubPicSize(SubPicInfo& info, std::pair<uint32_t, uint32_t> size);

    void RecLLSubPic(SubPicInfoMap& subPicInfo, const std::vector<SharedFrameBuffer>& subBandLL,
                     std::vector<std::pair<uint32_t, uint32_t>> recSubPicLLSize, uint32_t bitDepth);
    void RefLLSubPic(SubPicInfoMap& subPicInfo, const std::vector<SharedFrameBuffer>& subBandLL,
                     std::vector<std::pair<uint32_t, uint32_t>> recSubPicLLSize, std::vector<std::pair<uint32_t, uint32_t>> refSubPicLLSize, uint32_t bitDepth);

    Status WriteRecPic(const SharedBufferStorage& recPic,
                       const std::vector<std::pair<uint32_t, uint32_t>>& size,
                       std::ofstream& recFileHandle);

    void CopyRecPic(const SharedBufferStorage& recPic, const SharedBufferStorage& dstPic,
                    const std::vector<std::pair<uint32_t, uint32_t>>& size);

    Status RecImage(SubBandMap& subBands,
                    SubPicInfoMap& subPicInfo,
                    std::vector<SubPicInfoMap>& subPicInfoRec,
                    std::vector<SubPicInfoMap>& subPicLLInfoRec,
                    std::vector<SubPicInfoMap>& subPicLLInfoRef,
                    const SharedFrameBuffer& tmpRowBuffer,
                    const SharedFrameBuffer& tmpPicBuffer,
                    const std::vector<std::pair<uint32_t, uint32_t>>& picSize,
                    const std::vector<std::pair<uint32_t, uint32_t>>& picLLSize,
                    const std::string& recFile, const std::string& recLLFile, uint32_t bitDepth, bool needRef);
}

#endif // REC_IMAGE_H
