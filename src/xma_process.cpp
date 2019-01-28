
#include "xma_internal.h"
#include "xma_process.h"

namespace xma {

///-----------------------------ProcessMsgLienster---------------------------------------
ProcessMsgLienster::ProcessMsgLienster(Process *context, int fd, std::string name): EpollListener(fd, name), context_(context){
	
	XMA_DEBUG("CreateMsgLienster(): %s", Name().c_str());

	Epoll &epoll = context->GetContext().GetEpoll();
	epoll.Add(this);
	AddEvents(EPOLLIN | EPOLLERR | EPOLLHUP);
}

bool ProcessMsgLienster::DoHandle(void * data)
{
	struct epoll_event *ee = (epoll_event *)data;
	
	assert (this == ee->data.ptr);

	if (ee->events | EPOLLIN) {
#define MSG_CNT	1
		char *buff[MSG_CNT];
		int nr = read(GetFd(), &buff, sizeof(buff));
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

///---------------------------Process---------------------------------------------------------
Process::~Process()
{
	if (nullptr == rdlistener_)
		delete rdlistener_;
}

void Process::CreateMsgLienster() 
{
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
	
	rdlistener_ = new ProcessMsgLienster(this, msg_reader, Name() + "-msg_reader");
}	

void Process::Init() {
	//add the default service
	CreateMsgLienster();
	
	OnInit();
	
	for (auto &s: svcs_) 
		s->Init(this);
	
	XMA_DEBUG("Service initizated. size=%lu", svcs_.size());
}

bool Process::SendMsg(Msg *msg)
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

void Process::Main() {
#define MAX_EVENTS 1024
	int nfds;
	epoll_event events[MAX_EVENTS];
	while(IsRunning() && (nfds = GetContext().GetEpoll().Wait(events, MAX_EVENTS, 3)) != -1)
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
}
} //namespace xma
