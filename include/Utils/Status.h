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
#ifndef STATUS_H
#define STATUS_H

#include <istream>

namespace HFM {
    enum class Status {
        SUCCESS = 0,
        ERROR_UNKNOWN,
        INVALID_ARGUMENT,
        INVALID_FILE_HANDLE,
        PATH_NOT_FOUND,
        FILE_NOT_EXIST,
        NO_AVAILABLE_FRAME,
        ENCODER_RUNTIME_ERROR,
        ENCODE_TIME_OUT,
        DECODER_RUNTIME_ERROR,
        DECODE_TIME_OUT,
        INVALID_SUB_PIC_INFO,
        INCORRECT_CFG_FORMAT,
        CFG_VALUE_NOT_SET,
        INVALID_CFG_OPTION
    };

    static const char* GetStatusMsg(Status status) {
        switch (status) {
            case Status::SUCCESS:
                return "Success";
            case Status::ERROR_UNKNOWN:
                return "Unknown error";
            case Status::INVALID_ARGUMENT:
                return "Invalid argument";
            case Status::INVALID_FILE_HANDLE:
                return "Invalid file handle";
            case Status::PATH_NOT_FOUND:
                return "Path not found";
            case Status::FILE_NOT_EXIST:
                return "File not exit";
            case Status::NO_AVAILABLE_FRAME:
                return "No available frame";
            case Status::ENCODER_RUNTIME_ERROR:
                return "Encoder runtime error";
            case Status::ENCODE_TIME_OUT:
                return "Encode time out";
            case Status::DECODER_RUNTIME_ERROR:
                return "Decoder runtime error";
            case Status::DECODE_TIME_OUT:
                return "Decode time out";
            case Status::INVALID_SUB_PIC_INFO:
                return "Invalid sub-pic info";
            case Status::INCORRECT_CFG_FORMAT:
                return "Incorrect config format";
            case Status::CFG_VALUE_NOT_SET:
                return "Config value not set";
            case Status::INVALID_CFG_OPTION:
                return "Invalid config option";
        }
    }
}

#endif  // STATUS_H
