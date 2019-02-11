
// C/C++
#include <cassert>
#include <cstring>
#include <iostream>
#include <iomanip>

// Linux/POSIX
#include <sys/epoll.h>
#include <unistd.h>

// Internal
#include "xma_listener.h"
#include "xma_service.h"
#include "xma_process.h"

namespace xma {

ServiceList Service::services_;
std::mutex Service::mutex_;

void Service::List()
{
  std::unique_lock<std::mutex> lock(mutex_);

  std::cout << "Service number: " << services_.size() << std::endl;

#define _W (30)

  std::cout 
     << XMA_LEFT_OUTPUT(_W) << "Name" 
     << XMA_LEFT_OUTPUT(_W) << "Address"
     << XMA_LEFT_OUTPUT(_W) << "Context" 
     << std::endl;

  
  for (auto &s: services_) {    
    std::cout 
      << XMA_LEFT_OUTPUT(_W) << s->Name()  
      << XMA_LEFT_OUTPUT(_W) << s 
      << XMA_LEFT_OUTPUT(_W) << s->GetContext()->Name() 
      << std::endl;
  }
#undef _W

}
 
void Service::AddListener(Listener *l)
{ 
  listeners_.push_back(l);
}
void Service::RemoveListener(Listener *l)
{
  throw std::runtime_error("Remove listener at runtime is not supported.");
}

}
