#pragma once

#include <gccore.h>
#include <string>

namespace ThreadsProfiler {
    void init();
    void addThread(lwp_t thread, std::string name);
    s32 createThreadAndProfile(std::string name, lwp_t *thethread, void* (*entry)(void *), void *arg, void *stackbase, u32 stack_size, u8 prio);
}
