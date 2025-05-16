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

