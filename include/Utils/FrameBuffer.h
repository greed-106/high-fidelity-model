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
#ifndef FRAME_BUFFER_H
#define FRAME_BUFFER_H

#include <vector>
#include <memory>
#include "BasicTypes.h"

namespace ProVivid {
    using BufferStorage = std::vector<PelStorage>;
    using SharedBufferStorage = std::shared_ptr<BufferStorage>;
    using FrameBuffer = std::vector<Pel>;
    using SharedFrameBuffer = std::shared_ptr<FrameBuffer>;
    using PixelBuffer = std::vector<Pel>;
    using SharedPixelBuffer = std::shared_ptr<PixelBuffer>;
    using SubBandMap = std::unordered_map<int32_t, std::vector<SharedFrameBuffer>>;
}

#endif // FRAME_BUFFER_H

