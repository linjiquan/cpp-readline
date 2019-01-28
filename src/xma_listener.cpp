
// C/C++
#include <cassert>
#include <cstring>
#include <mutex>

// Linux/POSIX
#include <sys/epoll.h>
#include <unistd.h>

// Internal
#include "xma_internal.h"
#include "xma_listener.h"

namespace xma {

ListenerList Listener::listeners_;
std::mutex Listener::mutex_;

}

