
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

void Listener::List()
{
  std::unique_lock<std::mutex> lock(mutex_);

  std::cout << "Listener number: " << listeners_.size() << std::endl;

#define _W (30)

  std::cout 
     << XMA_LEFT_OUTPUT(_W) << "Name" 
     << XMA_LEFT_OUTPUT(_W) << "CtxName"
     << XMA_LEFT_OUTPUT(_W) << "Recv" 
     << XMA_LEFT_OUTPUT(_W) << "Succ" 
     << XMA_LEFT_OUTPUT(_W) << "Fail" 
     << std::endl;
  
  for (auto &s: listeners_) {
    std::cout 
      << XMA_LEFT_OUTPUT(_W) << s->Name() 
      << XMA_LEFT_OUTPUT(_W) << s->GetContainer()->Name() 
      << XMA_LEFT_OUTPUT(_W) << s->stats_.recv 
      << XMA_LEFT_OUTPUT(_W) << s->stats_.succ       
      << XMA_LEFT_OUTPUT(_W) << s->stats_.fail 
      << std::endl;
  }
#undef _W

  
}


}

