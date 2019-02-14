
// C/C++
#include <iostream>
#include <sstream>

//internal
#include "xma_thread.h"

namespace xma {


///------------------------------Thread Manager---------------------------------
ThreadList ThreadMgr::threads_;
std::mutex ThreadMgr::threads_mutex_;
std::atomic_int ThreadMgr::thread_seq_{0};

ThreadList threads = ThreadMgr::GetThreadList();

void ThreadMgr::List()
{
  std::unique_lock<std::mutex> lock(threads_mutex_);

  std::cout << "Thread number: " << threads_.size() << std::endl;

#define _W (30)

  std::cout 
     << XMA_LEFT_OUTPUT(_W) << "Id" 
     << XMA_LEFT_OUTPUT(_W) << "Name"
     << XMA_LEFT_OUTPUT(_W) << "Lcore"
     << XMA_LEFT_OUTPUT(_W) << "TID"
     << XMA_LEFT_OUTPUT(_W) << "SVCs"   
     << XMA_LEFT_OUTPUT(_W) << "State" 
     << std::endl;

  
  for (auto &t: threads_) {    
    std::cout 
      << XMA_LEFT_OUTPUT(_W) << t->Id()  
      << XMA_LEFT_OUTPUT(_W) << t->Name()  
      << XMA_LEFT_OUTPUT(_W) << t->GetRunningCore()
      << XMA_LEFT_OUTPUT(_W) << t->Tid()
      << XMA_LEFT_OUTPUT(_W) << t->GetServiceCount()
      << XMA_LEFT_OUTPUT(_W) << t->GetStrState()
      << std::endl;
  }
#undef _W

}


void ThreadMgr::Register(Thread* t)
{
  std::unique_lock<std::mutex> lock(threads_mutex_);
  threads_.push_back(t);
}

ThreadList ThreadMgr::GetThreadList()
{
  return threads_;
}

Thread *ThreadMgr::GetThread(std::string &name)
{
  for (auto &t: threads_) {
    if (t->Name() == name)
      return t;
  }

  return nullptr;
}

void ThreadMgr::Exit()
{
  for (auto &t: threads_)
    t->Stop();
}

uint32_t ThreadMgr::GetThreadId()
{
  ++thread_seq_;
  return static_cast<uint32_t>(thread_seq_);
}

///----------------------------------Basic Thread------------------------------------------
thread_local Thread* Thread::current_ = nullptr;

std::string Thread::GetStrState() const  {
  XMA_CASE_STR_BIGIN(GetState());
  XMA_CASE_STR(State::Stopped);
  XMA_CASE_STR(State::Running);
  XMA_CASE_STR(State::Sleeping);
  XMA_CASE_STR_END();
}


Thread::Thread(std::string name, int32_t lcore): name_(name), lcore_(lcore), running_(true)
{
  id_ = ThreadMgr::GetThreadId();
  
  SetState(State::Stopped);

  ThreadMgr::Register(this);

  thread_ = std::thread(&Thread::Run, this);
}

Thread::~Thread()
{
    if (thread_.joinable()) 
      thread_.join();
}

void Thread::SetAffinity() {
  cpu_set_t mask;
  CPU_ZERO(&mask);
  CPU_SET(lcore_, &mask);
  if (pthread_setaffinity_np(pthread_self(), sizeof(mask), &mask) < 0) {
    XMA_DEBUG("Set thread SetAffinity failed, name=%s, lcore=%d", Name().c_str(), lcore_);
    lcore_ = -1;
  }
}

uint64_t Thread::Tid() {
  std::stringstream ss;
  ss << std::this_thread::get_id();
  try {
    return std::stoull(ss.str());
  } catch (std::exception &e) {
    XMA_DEBUG("Can't get thread id, %s", e.what());
    return -1;
  }
}

void Thread::Run()
{
  current_ = this;
  
  SetAffinity();    
  Init();

  SetState(State::Running);
  
  while (running_) {
  Main();
  }

  SetState(State::Stopped);
}

} //namespace xma
