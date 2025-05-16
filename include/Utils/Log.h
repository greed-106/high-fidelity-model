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
#ifndef LOG_H
#define LOG_H

#ifdef DEBUG
#define LOGD(fmt, ...) \
do {                       \
    printf("ProVivid[D]:[%s:%d] " fmt, __func__, __LINE__, ##__VA_ARGS__); \
} while(0)
#else
#define LOGD(fmt, ...)
#endif

#define LOGI(fmt, ...) \
do {                       \
    printf("ProVivid[I]: " fmt, ##__VA_ARGS__); \
} while(0)

#define LOGW(fmt, ...) \
do {                       \
    printf("ProVivid[W]:[%s:%d] " fmt, __func__, __LINE__, ##__VA_ARGS__); \
} while(0)

#define LOGE(fmt, ...) \
do {                       \
    printf("ProVivid[E]:[%s:%d] " fmt, __func__, __LINE__, ##__VA_ARGS__); \
} while(0)

#endif // LOG_H

