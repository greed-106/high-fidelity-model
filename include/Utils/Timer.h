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

#ifndef TIMER_H
#define TIMER_H

#include <chrono>
#include <iomanip>
#include <iostream>
#include <string>
#include <utility>
#include <vector>
#include <unordered_map>

using TimePoint = decltype(std::chrono::high_resolution_clock::now());
using NanoSeconds = std::chrono::nanoseconds;

struct TimeRecord {
    TimeRecord() : startTimeSet(false), endTimeSet(false) {}
    bool startTimeSet;
    bool endTimeSet;
    TimePoint start;
    TimePoint end;
    std::string desc;
};

class Timer {
public:
    Timer() = default;
    explicit Timer(std::string functionName) : functionName_(std::move(functionName)) {}
    ~Timer()
    {
        Print();
    }

    void Start(std::string desc)
    {
        auto currTime = std::chrono::high_resolution_clock::now();
        if (desc_.find(desc) == desc_.end()) { // not found
            desc_.insert({desc, counter_});
            counter_++;
            TimeRecord record;
            record.desc = desc;
            record.start = currTime;
            record.startTimeSet = true;
            timeRecords_.emplace_back(record);
        } else {
            uint32_t pos = desc_.at(desc);
            timeRecords_.at(pos).start = currTime;
            timeRecords_.at(pos).startTimeSet = true;
        }
    };

    void End(std::string desc)
    {
        auto currTime = std::chrono::high_resolution_clock::now();
        if (desc_.find(desc) == desc_.end()) { // not found
            desc_.insert({desc, counter_});
            counter_++;
            TimeRecord record;
            record.desc = desc;
            record.end = currTime;
            record.endTimeSet = true;
            timeRecords_.emplace_back(record);
        } else {
            uint32_t pos = desc_.at(desc);
            timeRecords_.at(pos).end = currTime;
            timeRecords_.at(pos).endTimeSet = true;
        }
    };

    void Print()
    {

        const uint32_t C2 = 10;
        const std::string totalCostStr {" TotalCost "};
        uint32_t c1 = functionName_.length() - C2 - 2;
        for (auto v : timeRecords_) {
            if (v.startTimeSet && v.endTimeSet) {
                auto len = v.desc.length();
                if (len > c1) {
                    c1 = len;
                }
            }
        }
        c1 = totalCostStr.length() > c1 ? totalCostStr.length() : c1;
        c1 += 2;
        std::cout << "\n." << std::setfill('_') << std::setw(c1 + C2 + 2) << "." << std::endl;
        std::cout << "|" << std::setfill(' ') << std::setw(c1 + C2) << functionName_ << " |" << std::endl;
        std::cout << "|" << std::setfill('_') << std::setw(c1 + C2 + 2) << "|" << std::endl;
        std::cout << "|" << std::setfill(' ') << std::setw(c1) << " Module "
            << "|" << std::setw(C2) << " time(ms) " << "|" << std::endl;
        std::cout << "|" << std::setfill('_') << std::setw(c1 + 1) << "|"
            << std::setw(C2 + 1) << "|" << std::endl;
        std::cout << std::setfill(' ');
        float totalCost = 0.f;
        for (auto v : timeRecords_) {
            if (v.startTimeSet && v.endTimeSet) {
                NanoSeconds cost = std::chrono::duration_cast<NanoSeconds>(v.end - v.start);
                float timeCost = float(cost.count() / 1000000.f);
                totalCost += timeCost;
                std::cout << "|" << std::setw(c1) << " " + v.desc + " " << " | " << std::setw(C2 - 1)
                    << std::setprecision(3) << std::setiosflags(std::ios::fixed)
                    << timeCost << " |" << std::endl;
            }
        }
        std::cout << "|" << std::setfill('_') << std::setw(c1 + 1) << "|"
            << std::setw(C2 + 1) << "|" << std::endl;
        std::cout << "|" << std::setfill(' ') << std::setw(c1) << totalCostStr << "| "
            << std::setw(C2 - 1) << std::setprecision(3) << std::setiosflags(std::ios::fixed)
            << totalCost << " |" << std::endl;
        std::cout << "|" << std::setfill('_') << std::setw(c1 + 1) << "|"
            << std::setfill('_') << std::setw(C2 + 2) << "|\n" << std::endl;
    };

private:
    std::unordered_map<std::string, uint32_t> desc_;
    std::vector<TimeRecord> timeRecords_;
    uint32_t counter_ = 0;
    std::string functionName_;
};

#endif // TIMER_H

