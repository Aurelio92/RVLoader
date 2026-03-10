#pragma once

#include <gccore.h>
#include <mutex>
#include "rvlmutex.h"

class RVLConditionVariable {
private:
    cond_t _handle;

    // Prevent copying
    RVLConditionVariable(const RVLConditionVariable&) = delete;
    RVLConditionVariable& operator=(const RVLConditionVariable&) = delete;

public:
    RVLConditionVariable() {
        LWP_CondInit(&_handle);
    }

    ~RVLConditionVariable() {
        LWP_CondDestroy(_handle);
    }

    // Unblocks one of the threads waiting on this condition variable
    void notify_one() {
        LWP_CondSignal(_handle);
    }

    // Unblocks all threads waiting on this condition variable
    void notify_all() {
        LWP_CondBroadcast(_handle);
    }

    // Puts the thread to sleep.
    // Atomically unlocks the mutex and waits. Re-locks upon waking.
    void wait(std::unique_lock<RVLMutex>& lock) {
        LWP_CondWait(_handle, lock.mutex()->native_handle());
    }

    // Standard overload: Wait until the predicate is true.
    // This handles "spurious wakeups" automatically.
    template <typename Predicate>
    void wait(std::unique_lock<RVLMutex>& lock, Predicate pred) {
        while (!pred()) {
            wait(lock);
        }
    }
    
    // Accessor for the raw handle if needed elsewhere
    cond_t& native_handle() {
        return _handle;
    }
};