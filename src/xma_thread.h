#pragma once

// C/C++
#include <thread>
#include <vector>
#include <map>
#include <iostream>
#include <mutex>
#include <atomic>
#include <cassert>
#include <cstring>

// Linux/POSIX
#include <sys/epoll.h>
#include <unistd.h>

// Boost
#include <boost/lockfree/spsc_queue.hpp>

// Internal
#include "xma_internal.h"
#include "xma_context.h"
#include "xma_epoll.h"

namespace xma {

// Forward declarations
class Thread;
class Process;
class Worker;

using ThreadList = std::vector<Thread *>;

/// Thread manager class
class ThreadMgr
{
public:
	ThreadMgr() = default;
	~ThreadMgr() = default;

  static void Register(Thread* t);
  static ThreadList GetThreadList();
  static uint32_t GetThreadId();
  static void Exit();

private:
  static ThreadList threads_;
  static std::mutex threads_mutex_;
  static std::atomic_int thread_seq_;
};

class ThreadContext: public Context
{
public:
	ThreadContext() {}
	~ThreadContext() {}
	
	Epoll &GetEpoll() { return epoll_; }
private:
	Epoll epoll_;
};

class Thread
{
public:
  enum class State {
    Stopped = 0,    
    Running,       
    Sleeping
  };	


  Thread(std::string name, int32_t lcore);
  virtual ~Thread();

  virtual void Init() {};
  virtual void Main() {};

  void Stop() { running_ = false; }

  const std::string& Name() { 	return name_; }
  int32_t GetRunningCore() { return lcore_; }
  uint32_t Id() { return id_; }
  uint32_t Tid() { return tid_; }
  State GetState() { return state_; }
  void SetState(State state) { state_ = state; }
  ThreadContext &GetContext() { return context_; }
  bool IsRunning() { return running_; }
  
private:
  void Run();
  void SetAffinity();
  
private:
  ThreadContext context_;
  std::string name_;
  State state_;
  uint32_t id_;    // the logical thread id
  uint32_t tid_;   // the real thread id
  int32_t lcore_;  //the running logical core of this thread
  std::thread thread_;
  bool running_;   // the runing flag, only used for testing now
};

//
class Worker: public Thread
{
public:
	Worker(std::string name, int64_t cpu_set): Thread(name, cpu_set) {}

	virtual void Init()
	{
		XMA_DEBUG("Worker initializing: %s", Name().c_str());
	}

	virtual void Main() 
	{
		XMA_DEBUG("Worker running: %s", Name().c_str());
	    std::this_thread::sleep_for(std::chrono::seconds(2));
	}
private:

};
}
