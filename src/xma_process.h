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

	void Init();
	void Main();
	bool SendMsg(Msg *msg);
	void AddService(Service *svc) { svcs_.push_back(svc); }
	void CreateMsgLienster();

	virtual void OnInit()  {
		std::cout << "Basic Process OnInit()" << std::endl;
	}

  uint32_t GetServiceCount() override;
	
private:
	ServiceList svcs_;
  ProcessService *msg_svc_;
};
}
