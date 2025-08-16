#ifndef FAKE_DLFCN_H
#define FAKE_DLFCN_H

#include <stddef.h>

// 声明 fake_dlopen 函数
void *fake_dlopen(const char *libpath, int flags);

// 声明 fake_dlsym 函数
void *fake_dlsym(void *handle, const char *name);

// 声明 fake_dlclose 函数
int fake_dlclose(void *handle);

#endif // FAKE_DLFCN_H