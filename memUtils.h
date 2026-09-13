#pragma once
#include "logging.h"
#include <cstdlib>
#include <stdio.h>
#include <unistd.h>

#ifdef __arm__
#define A32
#elif defined __aarch64__
#define A64
#endif

#define LOG_RETURN_ADDRESS(level)                                              \
  do {                                                                         \
    uintptr_t ra = (uintptr_t)__builtin_return_address(level);                 \
    LOGI("Return address (offset): 0x%lx",                                     \
         g_libAddress ? (ra - g_libAddress) : ra);                             \
  } while (0)

extern uintptr_t g_libAddress;

uintptr_t getLibraryAddress(const char *libName);
uintptr_t getActualOffset(uintptr_t offset);

void PVZ2HookFunction(uintptr_t offset, void *replace, void **result);

void *copyVFTable(uintptr_t vftableAddr, int numVFuncs);
void patchVFTable(void *vftable, void *funcAddr, int index);
void copyVFTable(void *dest, uintptr_t vftableAddr, int numVFuncsToCopy);
void *createChildVFTable(int vFuncsCount, uintptr_t parentVftable,
                         int nuMVFuncsToCopy);
void setVFTable(void *obj, uintptr_t newVftablePtr);
void *GetVirtualFunc(void *obj, int index);

template <typename R, typename... Args>
R CallVirtualFunc(void *obj, int index, Args... args) {
  void *func = GetVirtualFunc(obj, index);
  auto castedFunc = reinterpret_cast<R (*)(Args...)>(func);
  return castedFunc(args...);
}
