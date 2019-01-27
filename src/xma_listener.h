#pragma once

//C/C++
#include <iostream>
#include <string>
#include <vector>
#include <mutex>
#include <list>
#include <thread>
#include <cassert>
#include <cstring>

//Linux/POSIX
#include <sys/epoll.h>
#include <unistd.h>
#include <fcntl.h>

namespace xma {

// Forward Declarations
class Listener;
class Service;
class Context;
class Epoll;
class Msg;
class MsgListener;
class ProcessEx;
class ProcessService;

using ListenerList = std::list<Listener *>;
using ServiceList = std::list<Service *>;


class Msg{
public:
	Msg(std::string sval): sval_(sval){		
	}

	~Msg() {}

	std::string &GetValue()
	{
		return sval_;
	}

private:
	uint16_t type_;
	std::string sval_;
};

class Epoll{
public:
	Epoll(): epoll_fd_(-1) {};
	~Epoll() {};

	void SetFd(int fd) { epoll_fd_ = fd; }
	int GetFd() { return epoll_fd_; }
private:
	int epoll_fd_;
};

class Context
{
public:
	Epoll &GetEpoll() { return epoll_; }
	
	bool AddEpollEvents(int fd, int events, void *data)
	{
		struct epoll_event ee;

//		memset (&ee, 0x00, sizeof(ee));

		ee.events = events;
		ee.data.ptr = data;
		
		if (-1 == epoll_ctl(GetEpoll().GetFd(), EPOLL_CTL_MOD, fd, &ee)) {
			std::cout << "Add epoll events faild: fd=" << fd << ";events=" << events << ";err=" << strerror(errno) << std::endl;
			return false;
		}

		std::cout << "Add epoll events: fd=" << fd << ";events=" << events << std::endl;

		return true;
	}

	bool AddEpollObj(int fd, void *obj) 
	{
		struct epoll_event ee;

	//	memset (&ee, 0x00, sizeof(ee));

		ee.data.ptr = obj;

		if (-1 == epoll_ctl(GetEpoll().GetFd(), EPOLL_CTL_ADD, fd, &ee)) {
			std::cout << "AddEpollObj failed: fd=" << fd << ";obj=" << obj << ";err" << strerror(errno) << std::endl;
			return false;
		}

		std::cout << "EpollFD added: fd=" << fd << ";obj=" << obj << std::endl;

		return true;
	}
	
	bool RemoveEpollObj(int fd) 
	{
		if (-1 == epoll_ctl(GetEpoll().GetFd(), EPOLL_CTL_DEL, fd, nullptr)) {
			std::cout << "RemoveEpollObj failed: fd=" << fd << ";err" << strerror(errno) << std::endl;
			return false;
		}

		std::cout << "RemoveEpollObj: fd=" << fd << std::endl;

		return true;
	}
private:
	Epoll epoll_;
};





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
	Listener(std::string name): listener_name_(name) {
		Register(this);
	}

	virtual ~Listener() {
		UnRegister(this);
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

private:
	static ListenerList listeners_;
	static std::mutex mutex_;
};

ListenerList Listener::listeners_;
std::mutex Listener::mutex_;

class EpollListener: public Listener {
public:
	EpollListener(std::string name): Listener(name) {}
};

class MsgListener: public Listener {

};

class TimerListener: public Listener {

};


/// The basic service class
class Service: public EpollListener{
public:
	Service(std::string name): EpollListener(name){
		name_ = name;
		Register(this);
	}

	virtual ~Service() {
		UnRegister(this);
	}

	void Init(Context *context) {
		std::cout << "Service init: " << Name() << " context: " << context << std::endl;
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

	void SaveContext(Context *context) {
		context_ = context;
	}

	Context * GetContext() {
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
	Context *context_;

	static ServiceList services_;
	static std::mutex mutex_;
};

ServiceList Service::services_;
std::mutex Service::mutex_;

class ProcessService: public Service{
public:
	ProcessService(std::string name): Service(name){
		std::cout << "ProcessService construct: " << Name() << std::endl;
	}
#define msg_writer msgconveyers_[1]
#define msg_reader msgconveyers_[0]

	void OnInit() override {
		std::cout << "OnInit(): " << Name() << std::endl;

		//create pipes for the inter process message
		//In the kernel, the pipe is marked writable in select/poll/epoll
		//only when the PIPE BUFFER is EMPTY, otherwise the pipe is ONLY marked
		//readable. So I can't use the non-block mode for the writer
		if (-1 == pipe (msgconveyers_)) {
			std::cout << "Create pipe2 failed " << strerror(errno) << std::endl;
			assert (0);
		}

		int val;
		if (-1 == (val = fcntl(msg_reader, F_GETFL, 0))) {
			std::cout << "Get msg reader option failed, err=" << strerror(errno) << std::endl;
			assert (0);
		}

		if (-1 == fcntl(msg_reader, F_SETFL, val | O_NONBLOCK)) {
			std::cout << "Set msg reader to non-block mode failed, err=" << strerror(errno) << std::endl;
			assert (0);
		}
		
		GetContext()->AddEpollObj(msg_reader, this);
//		GetContext()->AddEpollObj(msg_writer, this);
		GetContext()->AddEpollEvents(msg_reader, EPOLLIN | EPOLLERR | EPOLLHUP, this);
//		GetContext()->AddEpollEvents(msg_writer, EPOLLOUT| EPOLLERR | EPOLLHUP, this);
	}
	
	virtual bool DoHandle(void * data) override
	{
		struct epoll_event *ee = (epoll_event *)data;
		
		assert (this == ee->data.ptr);

		if (ee->events | EPOLLIN) {
	#define MSG_CNT	1
			char *buff[MSG_CNT];
			int nr = read(msg_reader, &buff, sizeof(buff));
			if (-1 == nr) {
				std::cout << "Read error: " << strerror(errno) << std::endl;
			} else if (nr % sizeof(buff[0])) {
				std::cout << "Invalid message received. bytes: " << nr << std::endl;
				return false;
			}

			int msg_cnt = nr / sizeof(buff[0]);
			for (int i = 0; i < msg_cnt; ++i) {
				Msg *msg = reinterpret_cast<Msg *>(buff[i]);
				std::cout << "Msg received: " << msg << std::endl;
				std::cout << "Msg value: " << msg->GetValue() << std::endl;
			}

			return true;


		} else if (ee->events | EPOLLOUT) {
		//	int fd = ev->data.fd;
		//	assert (fd == msg_writer);

		//		write (fd, "1", 1);
		} else {
			std::cout << "Unknow event received, event=" << ee->events << std::endl;
			return false;
		}
		
		return true;
	}

	bool SendMsg(Msg *msg) override
	{
		assert (msg != nullptr);

		//send the message pointer
		int n = write(msg_writer, &msg, sizeof(msg));
		if (-1 == n)
			return false;

		std::cout << "Msg send: " << &msg << " send size: " << n << std::endl;
		std::cout << "Msg value: " << msg->GetValue() << std::endl;
		
		return true;
	}

private:

	int msgconveyers_[2];
};


class ProcessEx
{
public:
	ProcessEx(std::string name, int lcore): name_(name), lcore_(lcore){
		std::cout << "ProcessEx construct..." << std::endl;
		thread_ = std::thread(&ProcessEx::Run, this);
	}

	~ProcessEx() {
		if (thread_.joinable())
			thread_.join();
	}

	static bool SendMsg (ProcessEx &proc, Msg *msg) {
		Service *svc = proc.GetDefaultServcie();
		if (svc == nullptr) {
			std::cout << "No default servcie" << std::endl;
			return false;
		}

		return svc->SendMsg(msg);
	}

	Service * GetDefaultServcie() {
		 if (svcs_.size() == 0)
			 return nullptr;

		 return svcs_.front();
	}

	void Init() {
		//set the lcore and so on
		int fd = epoll_create1(EPOLL_CLOEXEC);
		if (fd == -1) {
			std::cout << "epoll_create failed. err: " << strerror(errno) << std::endl;
			assert (0);
		}

		std::cout << "Epoll fd created: " << fd << std::endl;

		context_.GetEpoll().SetFd(fd);

		//add the default service
		AddService (new ProcessService("default"));

		OnInit();
	}

	void AddService(Service *svc) { svcs_.push_back(svc); }

	void Run() {
		Init();

#define MAX_EVENTS 1024
		
		int nfds;
		epoll_event events[MAX_EVENTS];

		std::cout << "Initizating servcies..." << std::endl;
		for (auto &s: svcs_) 
			s->Init(&context_);
		std::cout << "Servcie initizated. " << svcs_.size() << std::endl;

		std::cout << "ProcessEx runing: epoll_fd=" << context_.GetEpoll().GetFd() <<  "name=" << name_ << std::endl;

		while((nfds = epoll_wait(context_.GetEpoll().GetFd(), events, MAX_EVENTS, 3)) != -1)
		{
		  for (int i = 0; i < nfds; ++i )
		  {
			  EpollListener *l = reinterpret_cast<EpollListener *>(events[i].data.ptr);
			  if (l == nullptr) {
				  std::cout << "Invalid epoll event: fd=" << events[i].data.fd << ";ptr=" << events[i].data.ptr << std::endl;
				  continue;
			  }
		
			  std::cout << "l: "  << l << ";ptr: " << events[i].data.ptr << std::endl;

			  l->Handle(&events[i]);
		  }
		}

		std::cout << "Epoll result : " << nfds << " last error: " << strerror(errno) << std::endl;
		std::cout << "Thread existed: " << name_ << std::endl;
	}

	virtual void OnInit() {
		std::cout << "Basic ProcessEx OnInit()" << std::endl;
	}

private:
	std::string name_;
	int lcore_;
	Context context_;
	std::thread thread_;
	ServiceList svcs_;
};




}

