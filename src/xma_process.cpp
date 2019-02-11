
#include "xma_internal.h"
#include "xma_process.h"

namespace xma {

///-----------------------------ProcessMsgLienster---------------------------------------
ProcessMsgListener::ProcessMsgListener(std::string name, ListenerContainer c, int fd): EpollListener(name, c, fd)
{
	
	XMA_DEBUG("ProcessMsgLienster(): %s", Name().c_str());

	Epoll &epoll = c->GetContext()->GetContext().GetEpoll();
	epoll.Add(this);
	AddEvents(EPOLLIN | EPOLLERR | EPOLLHUP);
}

ProcessMsgListener::~ProcessMsgListener() {
	XMA_DEBUG("~ProcessMsgLienster(): %s", Name().c_str());
}

void ProcessMsgListener::Dispatch(Msg *msg)
{
  XMA_DEBUG("%s: msg received: %p, %s", Name().c_str(), (void *)msg, msg->GetValue().c_str());
  XMA_DEBUG("%s: free msg: %p", Name().c_str(), (void *)msg);  
  delete msg;
}

bool ProcessMsgListener::DoHandle(void * data)
{
	struct epoll_event *ee = (epoll_event *)data;
	
	assert (this == ee->data.ptr);

	if (ee->events | EPOLLIN) {
#define MSG_CNT	1
		char *buff[MSG_CNT];
		int nr = read(GetFd(), &buff, sizeof(buff));
		if (-1 == nr) {
			XMA_DEBUG("Read error: %s, err=%s", Name().c_str(), strerror(errno));
            return false;
		} else if (nr % sizeof(buff[0])) {
			XMA_DEBUG("Invalid message received: %s, bytes=%d", Name().c_str(), nr);
			return false;
		}

		int msg_cnt = nr / sizeof(buff[0]);
		for (int i = 0; i < msg_cnt; ++i) {
			Msg *msg = reinterpret_cast<Msg *>(buff[i]);
			Dispatch(msg);
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

///-----------------------------Process service---------------------------------------
ProcessService::ProcessService(std::string name): Service(name), rdlistener_(nullptr)
{
}

ProcessService::~ProcessService()
{
  // msg_reader is managered by the rdlistener_  
	if (msg_writer)
		::close(msg_writer);

  if (rdlistener_ != nullptr) {
    RemoveListener(rdlistener_);
    delete rdlistener_;
  }
}

void ProcessService::OnInit()
{
  assert (rdlistener_ == nullptr);
  
  CreateMsgConveyers();
  
  rdlistener_ = new ProcessMsgListener(Name(), this, msg_reader);
  AddListener(rdlistener_);
}


void ProcessService::CreateMsgConveyers() 
{
	//create pipes for the inter process message
	//In the kernel, the pipe is marked writable in select/poll/epoll
	//only when the PIPE BUFFER is EMPTY, otherwise the pipe is ONLY marked
	//readable. So I can't use the non-block mode for the writer
	if (-1 == pipe (msgconveyers_)) {
		throw std::runtime_error(std::string("Failed to create process msg listener, name:") + 
			Name() + " err:" + strerror(errno));
	}

	int val;
	if (-1 == (val = fcntl(msg_reader, F_GETFL, 0))) {
		throw std::runtime_error(std::string("Failed to create process msg listener, name:") + 
			Name() + " err:" + strerror(errno));
	}

	if (-1 == fcntl(msg_reader, F_SETFL, val | O_NONBLOCK)) {
		throw std::runtime_error(std::string("Failed to create process msg listener, name:") + 
			Name() + " err:" + strerror(errno));
	}
}

bool ProcessService::SendMsg(Msg *msg)
{
	if (msg == nullptr)
		return true;

	//send the message pointer
	int n = write(msg_writer, &msg, sizeof(msg));
	if (-1 == n) {
		XMA_DEBUG("Send msg failed: %p, err: %s", static_cast<void *>(msg), strerror(errno));
		return false;
	}

	XMA_DEBUG("Send msg done: %p, size: %d", static_cast<void *>(msg), n);
	
	return true;  
}




///---------------------------Process-------------------------------------------------
Process::~Process()
{
  if (proc_svc_ != nullptr)
    delete proc_svc_;
}

uint32_t Process::GetServiceCount()
{
  return svcs_.size();
}

void Process::Init() {
	XMA_DEBUG("[%s]Process init...", Name().c_str());

	//add the default service
	proc_svc_ = new ProcessService(Name() + "/service*");
	XMA_DEBUG("[%s] default service is %p", Name().c_str(), (void *)proc_svc_);

  AddService(proc_svc_);
  
	OnInit();
	
	for (auto &s: svcs_) 
		s->Init(this);

	RegisterCommand();
	
	XMA_DEBUG("[%s]Service started: size=%lu", Name().c_str(), svcs_.size());
}

bool Process::SendMsg(Msg *msg)
{
  assert (proc_svc_ != nullptr);
  return proc_svc_->SendMsg(msg);
}

TimerServer &Process::GetTimerServer()
{
	assert (proc_svc_ != nullptr);
	return proc_svc_->timer_server_;
}

bool Process::SetTimer(Timer *t)
{
	assert (proc_svc_ != nullptr);

	return proc_svc_->timer_server_.SetTimer(t);
}

bool Process::StopTimer(Timer *t)
{
	assert (proc_svc_ != nullptr);

	return proc_svc_->timer_server_.StopTimer(t);
}
bool Process::RestartTimer(Timer *t)
{
	assert (proc_svc_ != nullptr);

	if (StopTimer(t) != true)
		return false;
	
	return SetTimer(t);
}

void Process::Main() {
#define MAX_EVENTS 1024
	int nfds;
	int timeout = 3;

	epoll_event events[MAX_EVENTS];
	while(IsRunning() && (nfds = GetContext().GetEpoll().Wait(events, MAX_EVENTS, timeout)) != -1)
	{
		timeout = std::min(static_cast<int>(proc_svc_->timer_server_.CheckTimers().count()), timeout);
		if (timeout <= 0)
			timeout = 3;

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
}
} //namespace xma
