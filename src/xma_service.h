#pragma once 

// C/C++
#include <list>
#include <string>

// Internal
#include "xma_listener.h"
#include "xma_message.h"

namespace xma {

/// Forward Declarations
class Service;
class Process;
class Listener;
class Socket;

using ServiceList = std::list<Service *>;

class ServiceContext: public Context
{
public:
	ServiceContext() {}
private:
	
};

/// The basic service class
class Service {
public:
	Service(std::string name) {
		name_ = name;
		Register(this);
	}

	virtual ~Service() {
		UnRegister(this);
	}

  void Init(Process *context) {
    XMA_DEBUG("Service init: %s", name_.c_str());
    SaveContext(context);
    OnInit();
  }

	virtual bool OnSocketErr(Socket *s)
	{
		XMA_DEBUG("[%s] Socket error, socket=%p", Name().c_str(), (void *)s);
		return true;
	}

  virtual void OnInit() {
    std::cout << "The basic OnInit()" << std::endl;
  }

	void SaveContext(Process *context) {
		context_ = context;
	}

	Process * GetContext() {
		return context_;
	}

  uint32_t GetListenerCount() const;
  
  void AddListener(Listener *l);
  void RemoveListener(Listener *l);
  
	std::string &Name() { return name_; }
	
	static void Register(Service *s) {
		std::unique_lock<std::mutex> lock(mutex_);
		services_.push_back(s);
	}

	static void UnRegister(Service *s) {
		std::unique_lock<std::mutex> lock(mutex_);
		services_.remove(s);
	}

  static void List(); 

private:
	std::string name_;
	Process *context_;

  std::vector<Listener *> listeners_;

	static ServiceList services_;
	static std::mutex mutex_;
};

//ServiceList Service::services_;
//std::mutex Service::mutex_;

}
