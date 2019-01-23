#pragma once

#include <vector>
#include <thread>
#include <mutex>
#include <atomic>


namespace xma {

// This is the IO Event driving kernel. Reactor model.
// This class is a wrapper of event_base but not only a wrapper.
// It provides a simple way to run a IO Event driving loop.
// One thread one loop.
class ThreadLooper {
public:
    ThreadLooper() {
        evbase_ = event_base_new();
    }
    
    ~ThreadLooper() {
        event_base_free(evbase_);
        evbase_ = nullptr;
    }
    
    void Dispatch();

private:
    void Init();
private:
    struct event_base* evbase_;
};
}
