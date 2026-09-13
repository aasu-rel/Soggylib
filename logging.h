#pragma once
#include <android/log.h>

#define LIB_TAG "Soggy"

#define LOGI(...)                                                              \
  ((void)__android_log_print(ANDROID_LOG_INFO, LIB_TAG, __VA_ARGS__))
#define LOGW(...)                                                              \
  ((void)__android_log_print(ANDROID_LOG_WARN, LIB_TAG, __VA_ARGS__))
#define LOGE(...)                                                              \
  ((void)__android_log_print(ANDROID_LOG_ERROR, LIB_TAG, __VA_ARGS__))
