#pragma once 

// C/C++
#include <list>
#include <string>

// Internal
#include "xma_internal.h"
#include "xma_context.h"
#include "xma_message.h"

namespace xma {

/// Forward Declarations
class Service;
class Process;

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

	virtual bool SendMsg(Msg *msg) {
		std::cout << "The basic SendMsg is not implemented." << std::endl;
		return false;
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
	
	std::string &Name() { return name_; }
	
	static void Register(Service *s) {
		std::unique_lock<std::mutex> lock(mutex_);
		services_.push_back(s);
	}

	static void UnRegister(Service *s) {
		std::unique_lock<std::mutex> lock(mutex_);
		services_.remove(s);
	}

private:
	std::string name_;
	Process *context_;

	static ServiceList services_;
	static std::mutex mutex_;
};

//ServiceList Service::services_;
//std::mutex Service::mutex_;

}
