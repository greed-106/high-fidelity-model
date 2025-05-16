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
#ifndef STATUS_H
#define STATUS_H

#include <istream>

namespace ProVivid {
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
