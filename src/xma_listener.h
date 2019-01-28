#pragma once

// C/C++
#include <iostream>
#include <string>
#include <vector>
#include <mutex>
#include <list>
#include <thread>
#include <cassert>
#include <cstring>

// Linux/POSIX
#include <sys/epoll.h>
#include <unistd.h>
#include <fcntl.h>

// Internal
#include "xma_internal.h"
#include "xma_context.h"
#include "xma_message.h"

namespace xma {

// Forward Declarations
class Listener;
class Service;
class Epoll;
class MsgListener;
class ProcessEx;
class ProcessService;

using ListenerList = std::list<Listener *>;


struct ListenerStats
{
	ListenerStats(): recv(0), succ(0), fail(0) {}
	uint64_t recv;
	uint64_t succ;
	uint64_t fail;
};

// The basic Listener class
class Listener {
public:
	Listener(std::string name): listener_name_(name), context_(nullptr) {
		Register(this);
	}

	virtual ~Listener() {
		UnRegister(this);
	}

	void SetContext(Context *context) {
		context_ = context;
	}		
	
	Context * GetContext() {
		return context_;
	}

	std::string &Name() { 
		return listener_name_;
	}
	
	bool Handle(void *data) {
		IncRecv();
		
		if (DoHandle(data)) {
			IncSucc();
			return true;
		} else {
			IncFail();	
			return false;
		}
	}
	
	virtual bool DoHandle(void * data) {
		std::cout << "Basic DoHandle" << std::endl;
		return true;
	}

	static void Register(Listener *l) { 
		std::unique_lock<std::mutex> lock(mutex_);
		listeners_.push_back(l);
	}

	static void UnRegister(Listener *l) {
		std::unique_lock<std::mutex> lock(mutex_);
		listeners_.remove(l);
	}

	static int Size() {
		std::unique_lock<std::mutex> lock(mutex_);
		return listeners_.size();
	}
	
	std::string &GetListenerName() { return listener_name_; }

	void IncRecv() { stats_.recv++; }
	void IncSucc() { stats_.succ++; }
	void IncFail() { stats_.fail++; }
private:
	ListenerStats stats_;
	std::string listener_name_;
	Context * context_;

private:
	static ListenerList listeners_;
	static std::mutex mutex_;
};

//ListenerList Listener::listeners_;
//std::mutex Listener::mutex_;
}

