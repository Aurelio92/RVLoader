#pragma once

#include <gccore.h>
#include <mutex>

class RVLMutex {
private:
    mutex_t _handle;

    //Prevent copying, as mutexes should not be copied
    RVLMutex(const RVLMutex&) = delete;
    RVLMutex& operator=(const RVLMutex&) = delete;

public:
    RVLMutex() {
        //Initialize the libogc mutex
        //'false' indicates it is not recursive by default (standard std::mutex behavior)
        LWP_MutexInit(&_handle, false); 
    }

    ~RVLMutex() {
        //Clean up resources
        LWP_MutexDestroy(_handle);
    }

    //Required for BasicLockable
    void lock() {
        LWP_MutexLock(_handle);
    }

    //Required for BasicLockable
    void unlock() {
        LWP_MutexUnlock(_handle);
    }

    //Required for Lockable (needed for std::unique_lock::try_lock)
    bool try_lock() {
        //LWP_MutexTryLock returns 0 on success
        return (LWP_MutexTryLock(_handle) == 0);
    }

    mutex_t& native_handle() {
        return _handle;
    }
};