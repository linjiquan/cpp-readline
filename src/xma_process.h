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
#include "xma_internal.h"
#include "xma_service.h"
#include "xma_thread.h"

namespace xma {
class ProcessMsgLienster: public EpollListener
{
public:
	ProcessMsgLienster(Process *context, int fd, std::string name);
	bool DoHandle(void * data) override;
private:
	Process *context_;
};
	
class Process: public Thread
{
public:
	Process(std::string name, int32_t cpu_set): Thread(name, cpu_set), rdlistener_(nullptr) {}
	virtual ~Process();

	void Init();
	void Main();
	bool SendMsg(Msg *msg);
	void AddService(Service *svc) { svcs_.push_back(svc); }
	void CreateMsgLienster();

	virtual void OnInit() {
		std::cout << "Basic Process OnInit()" << std::endl;
	}
	
#define msg_writer msgconveyers_[1]
#define msg_reader msgconveyers_[0]	
private:
	ServiceList svcs_;
	ProcessMsgLienster * rdlistener_; //message reader listener
	//ProcessMsgLienster * wdlistener_; //message writer listener
	int msgconveyers_[2];		
};
}
