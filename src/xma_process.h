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


// Internal
#include "xma_listener.h"
#include "xma_epoll.h"
#include "xma_service.h"
#include "xma_thread.h"
#include "xma_message.h"
#include "xma_timer.h"
#include "xma_shell.h"

namespace xma {


class ProcessMsgListener: public EpollListener
{  
public:
  ProcessMsgListener(std::string name, ListenerContainer c, int fd);
  ~ProcessMsgListener();
  
  void Dispatch(Msg *);
  bool DoHandle(void * data) override;
};

// each process should have one default process service
// used to receive msg, timer check and so on
class ProcessService: public Service
{
public:
  ProcessService(std::string name);
  ~ProcessService();

  bool SendMsg(Msg *msg);

//  int SetTimer(Duration d, void *data);
//  void StopTimer(int timer_id);

  TimerMgr timer_mgr_;

private:
  void OnInit();
  void CreateMsgConveyers(); 
#define msg_writer msgconveyers_[1]
#define msg_reader msgconveyers_[0] 
  
private:
  ProcessMsgListener *rdlistener_;
  int msgconveyers_[2];
};

class Process: public Thread
{
public:
  Process(std::string name, int32_t cpu_set): Thread(name, cpu_set) {}
  virtual ~Process();

  virtual void OnShell(Shell &shell) {}

  void Init() override;
  void Main() override;
  
  bool SendMsg(Msg *msg) override;

  bool SetTimer(Timer *t);
  bool StopTimer(Timer *t);
  bool RestartTimer(Timer *t);
  
  void AddService(Service *svc) { svcs_.push_back(svc); }
  void CreateMsgLienster();
  TimerMgr &GetTimerMgr();

  uint32_t GetServiceCount() override;
  
private:
  ServiceList svcs_;
  ProcessService *proc_svc_;//default service
};

}
