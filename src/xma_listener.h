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
#include "xma_service.h"

namespace xma {

// Forward Declarations
class Listener;
class Service;
class Epoll;

using ListenerList = std::list<Listener *>;
using ListenerContainer = Service *;

struct ListenerStats
{
	uint64_t recv;
	uint64_t succ;
	uint64_t fail;
};

// Listener is used to receive data in service, each Listener should 
// be contained in one service as its container
// Compared to use polymorphism, it is better to use the intra-class:
// 1. To avoid Naming conflict
// 2. Easy to contain Thousands of listeners in one parent container
class Listener {
public:
	Listener(std::string name, ListenerContainer container): listener_name_(name), container_(container) {
    memset (&stats_, 0x00, sizeof(stats_));
		Register(this);
	}

	virtual ~Listener() {
		UnRegister(this);
	}

	void SetContainer(ListenerContainer container) {
		container_ = container;
	}		
	
	ListenerContainer GetContainer() {
		return container_;
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
		XMA_DEBUG("[%s] Register %p, size=%lu", l->Name().c_str(), (void *)l, listeners_.size());
	}

	static void UnRegister(Listener *l) {
		std::unique_lock<std::mutex> lock(mutex_);
		listeners_.remove(l);
		XMA_DEBUG("[%s] Unregister %p, size=%lu", l->Name().c_str(), (void *)l, listeners_.size());
	}

	static int Size() {
		std::unique_lock<std::mutex> lock(mutex_);
		return listeners_.size();
	}
	
	std::string &GetListenerName() { return listener_name_; }

  static void List();

	void IncRecv() { stats_.recv++; }
	void IncSucc() { stats_.succ++; }
	void IncFail() { stats_.fail++; }
private:
	ListenerStats stats_;
	std::string listener_name_;
	ListenerContainer container_;

private:
	static ListenerList listeners_;
	static std::mutex mutex_;
};

//ListenerList Listener::listeners_;
//std::mutex Listener::mutex_;
}

