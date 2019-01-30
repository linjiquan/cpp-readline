
// C/C++
#include <cassert>
#include <cstring>

// Linux/POSIX
#include <sys/epoll.h>
#include <unistd.h>

// Internal
#include "xma_internal.h"
#include "xma_service.h"

namespace xma {

ServiceList Service::services_;
std::mutex Service::mutex_;

}

