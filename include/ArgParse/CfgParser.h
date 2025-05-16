#ifndef CFG_PARSER_H
#define CFG_PARSER_H

#include <string>
#include <fstream>
#include "ArgEncoder.h"
#include "Log.h"
#include "Status.h"
#include "Utils.h"

namespace ProVivid {
    class CfgParser {
    public:
        CfgParser(int argc, const char** argv)
        {
            auto GetArgIndex = [argc, &argv](const std::string& option) -> int {
                for (int i = 0; i < argc; ++i) {
                    if (std::string(argv[i]) == option) {
                        return i;
                    }
                }
                return -1;
            };
            int index = GetArgIndex("-c");
            if (index > 0) {
                cfgPath_ = std::string(argv[index + 1]);
            }
        };

        ~CfgParser() = default;

        void UpdateArgs(Arguments args)
        {
            auto allEntries = args.get_all_entries();
            auto argNames = args.argNames;
            this->Parse(argNames); // write cfg args to cfgArgs_
            for (auto & v : cfgArgs_) {
                for (auto & entry : allEntries) {
                    auto keyEntry = entry->get_keys();
                    if (std::find(keyEntry.begin(), keyEntry.end(), v.first) != keyEntry.end() &&
                        !entry->set_by_user()) {
                        entry->set_value(v.second);
                        entry->set_datap_value(v.second);
                    }
                }
            }
        }

        std::unordered_map<std::string, std::string> cfgArgs_;

    private:
        void Parse(std::vector<std::string>& argNames)
        {
            if (cfgPath_.empty()) {
                return;
            }
            std::ifstream cfgFile(cfgPath_);
            std::string line;
            if (cfgFile.fail()) {
                LOGE("failed to open: %s\n", cfgPath_.c_str());
            } else {
                while (getline(cfgFile, line)) {
                    currLine_++;
                    ParseLine(line, argNames);
                }
                cfgFile.close();
            }
        }

        Status ParseLine(const std::string& line, std::vector<std::string>& argNames)
        {
            auto validStr = SplitString(line, "#")[0];
            if (IsAllWhiteSpace(validStr)) {
                return Status::SUCCESS;
            }
            if (!validStr.empty() && validStr.find(':')) {
                auto opt = SplitString(validStr, ":");
                if (opt.size() <= 1) {
                    auto status = Status::INCORRECT_CFG_FORMAT;
                    LOGE("%s in line: %d\n", GetStatusMsg(status), currLine_);
                    return status;
                } else {
                    auto key = Trim(opt[0]);
                    if (std::find(argNames.begin(), argNames.end(), key) != argNames.end()) {
                        if (opt.size() == 1) {
                            auto status = Status::CFG_VALUE_NOT_SET;
                            LOGE("%s for key \"%s\" in line: %d\n", GetStatusMsg(status), key.c_str(), currLine_);
                            return status;
                        }
                        auto value = Trim(opt[1]);
                        if (IsAllWhiteSpace(value)) {
                            auto status = Status::CFG_VALUE_NOT_SET;
                            LOGE("%s for key \"%s\" in line: %d\n", GetStatusMsg(status), key.c_str(), currLine_);
                            return status;
                        }
                        cfgArgs_[key] = value;
                    } else {
                        auto status = Status::INVALID_CFG_OPTION;
                        LOGE("%s \"%s\" in line: %d\n", GetStatusMsg(status), key.c_str(), currLine_);
                        return status;
                    }
                }
            }
            return Status::SUCCESS;
        };
        std::string cfgPath_;
        uint32_t currLine_{0};
    };
}

#endif  // CFG_PARSER_H
