#include "memUtils.h"
#include "And64Inline/And64InlineHook.hpp"
#include <cerrno>
#include <cstring>
#include <mutex>

uintptr_t g_libAddress = 0;

uintptr_t getLibraryAddress(const char *libName) {
  FILE *fp = fopen("/proc/self/maps", "rt");
  if (fp == NULL) {
    LOGE("Failed to open /proc/self/maps: %s", strerror(errno));
    return 0;
  }

  uintptr_t addr = 0;
  char line[512];

  while (fgets(line, sizeof(line), fp)) {
    // Match exact library name to avoid partial matches (e.g., libPVZ2 vs
    // libPVZ2_other)
    if (strstr(line, libName) && strstr(line, ".so")) {
      // Parse the first hex address (start of the mapped region)
      addr = strtoull(line, NULL, 16);
      break;
    }
  }

  fclose(fp);
  return addr;
}

static std::once_flag g_libInitFlag;

uintptr_t getActualOffset(uintptr_t offset) {
  std::call_once(g_libInitFlag, []() {
    g_libAddress = getLibraryAddress("libPVZ2.so");
    if (g_libAddress == 0) {
      LOGE("Failed to locate libPVZ2.so");
    } else {
      LOGI("libPVZ2.so base = 0x%lx", g_libAddress);
    }
  });
  return g_libAddress + offset;
}

void PVZ2HookFunction(uintptr_t offset, void *replace, void **result) {
  void *target = (void *)getActualOffset(offset);
  if (target == nullptr) {
    LOGE("PVZ2HookFunction: Invalid target for offset 0x%lx", offset);
    return;
  }
#ifdef A32
  MSHookFunction(target, replace, result);
#else
  A64HookFunction(target, replace, result);
#endif
}