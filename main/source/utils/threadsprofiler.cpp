#include <string>
#include <malloc.h>
#include <gccore.h>
#include <unistd.h>
#include <ogc/lwp_threads.h>
#include <ogc/lwp_objmgr.h>
#include <unordered_map>
#include <map>
#include "threadsprofiler.h"

extern "C" {
    void __thread_dispatch(void);
    void __real___thread_dispatch(void);
}

extern lwp_cntrl *_thr_heir;

namespace ThreadsProfiler {
    bool initialized = false;
    u64 initTime;
    std::unordered_map<s32, std::string> threads;

    #define PROF_BUF_SIZE 256

    typedef struct {
        u32 thread_id;
        u64 timestamp;
    } DispatchEntry;

    static volatile struct {
        DispatchEntry data[PROF_BUF_SIZE];
        u32 head; // Written by Dispatch
        u32 tail; // Read by Profiler
    } profiler_queue = { .head = 0, .tail = 0 };

    u32 computeAddressFromBL_PPC(u32 src, u32 opcode) {
        u32 jump = opcode & 0x03FFFFFC;
        u32 jump_extended = (jump & 0x02000000) ? (jump | 0xFC000000) : jump;
        return (src + jump_extended);
    }

    u32 computeBL(u32 src, u32 dst) {
        u32 bl = (dst - src);
        bl &= 0x03FFFFFC;
        bl |= 0x48000001;
        return bl;
    }

    void findCallersAndPatch(u32 dst, u32 newDest) {
        u32* toPatch[32] = {0};
        u32 patchCount = 0;
        u32* src = (u32*)0x81330000;
        while (src != (u32*)0x81340000) {
            if (computeAddressFromBL_PPC((u32)src, *src) == dst) {
                printf("Found caller for %08X at @%08X\n", dst, (u32)src);
                toPatch[patchCount] = src;
                patchCount++;
            }
            src++;
        }

        for (u32 i = 0; i < patchCount; i++) {
            *toPatch[i] = computeBL((u32)toPatch[i], newDest);
            DCFlushRange((void*)toPatch[i], sizeof(u32));
            ICInvalidateRange((void*)toPatch[i], sizeof(u32));
        }
    }

    void myThreadDispatch() {
        if (initialized) {
            static s32 lastDispatchedThread = -1;

            s32 newId = _thr_heir->object.id;

            if (threads.count(newId) && newId != lastDispatchedThread) {
                // printf("[THREAD PROFILER]. %u %s\n", diff_msec(initTime, gettime()), threads[newId].c_str());
                u32 next_head = (profiler_queue.head + 1) % PROF_BUF_SIZE;

                // Only write if the buffer isn't full (avoiding locks)
                if (next_head != profiler_queue.tail) {
                    profiler_queue.data[profiler_queue.head].thread_id = newId;
                    profiler_queue.data[profiler_queue.head].timestamp = gettime();
                    profiler_queue.head = next_head;
                }
                lastDispatchedThread = newId;
            }
        }
        __real___thread_dispatch();
    }

    void* profiler_thread_func(void* arg) {
        while (1) {
            while (profiler_queue.tail != profiler_queue.head) {
                // Fix the volatile binding error
                volatile DispatchEntry* ve = &profiler_queue.data[profiler_queue.tail];
                DispatchEntry entry;
                entry.thread_id = ve->thread_id;
                entry.timestamp = ve->timestamp;

                u32 ms = diff_msec(initTime, entry.timestamp);

                //printf("[PROF] Thread %s dispatched at %u ms\n", threads[entry.thread_id].c_str(), ms);

                profiler_queue.tail = (profiler_queue.tail + 1) % PROF_BUF_SIZE;
            }
            usleep(20000); // 20ms sleep
        }
        return NULL;
    }

    void init() {
        //findCallersAndPatch((u32)__thread_dispatch, (u32)myThreadDispatch);
        initTime = gettime();
        initialized = true;
        void* stack = memalign(32, 16*1024);
        lwp_t thethread;
        LWP_CreateThread(&thethread, profiler_thread_func, NULL, stack, 16*1024, 80);
    }

    void addThread(lwp_t thread, std::string name) {
        threads[(s32)LWP_OBJMASKID(thread)] = name;
    }
    
    s32 createThreadAndProfile(std::string name, lwp_t *thethread, void* (*entry)(void *), void *arg, void *stackbase, u32 stack_size, u8 prio) {
        s32 ret = LWP_CreateThread(thethread, entry, arg, stackbase, stack_size, prio);
        if (ret != 0) {
            return ret;
        }
        
        addThread(*thethread, name);
        
        return 0;
    }
}

extern "C" {
    void __wrap___thread_dispatch() {
        ThreadsProfiler::myThreadDispatch();
    }
}