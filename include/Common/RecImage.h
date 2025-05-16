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
