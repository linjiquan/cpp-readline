
#include "xma_thread.h"

namespace xma {


///------------------------------Thread Manager---------------------------------
ThreadList ThreadMgr::threads_;
std::mutex ThreadMgr::threads_mutex_;
std::atomic_int ThreadMgr::thread_seq_{0};

void ThreadMgr::Register(Thread* t)
{
  std::unique_lock<std::mutex> lock(threads_mutex_);
  threads_.push_back(t);
}

ThreadList ThreadMgr::GetThreadList()
{
  return threads_;
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

///----------------------------------Basic Thread------------------------------
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

void Thread::Run()
{
  SetAffinity();	  
  Init();

  SetState(State::Running);
  
  while (running_) {
	Main();
  }

  SetState(State::Stopped);
}

} //namespace xma
