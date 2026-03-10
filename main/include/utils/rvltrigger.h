#pragma once
#include <atomic>

class RVLTrigger {
private:
    // std::atomic guarantees thread safety without heavy locking
    std::atomic<bool> _flag;

public:
    RVLTrigger() : _flag(false) {}

    // Called by the Sender
    void send() {
        _flag.store(true);
    }

    // Called by the Looping Thread
    // Returns true if the signal was sent, and automatically resets it to false.
    bool check() {
        // 'exchange' is crucial here. It reads the value AND sets it to false
        // in one atomic step. This prevents you from processing the same 
        // signal twice.
        return _flag.exchange(false);
    }
    
    // Optional: Check without resetting (if you just want to peek)
    bool is_set() const {
        return _flag.load();
    }
};