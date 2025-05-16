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

