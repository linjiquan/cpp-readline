
// C/C++
#include <cassert>
#include <cstring>

// Linux/POSIX
#include <sys/epoll.h>
#include <unistd.h>

// Internal
#include "xma_internal.h"
#include "xma_epoll.h"
#include "xma_process.h"

namespace xma {
///------------------------------------------------EpollLienster-----------------------------------------
	EpollListener::EpollListener(std::string name, ListenerContainer c, int fd): Listener(name, c), fd_(fd), epoll_(nullptr) 
	{
	}
	
	EpollListener::EpollListener(std::string name, ListenerContainer c): Listener(name, c), epoll_(nullptr) 
	{
    fd_ = -1;
	}


	EpollListener::~EpollListener() 
	{ 
    Close();
	}
  
  void EpollListener::Close()
  {
		if (fd_ != -1 && epoll_) {
      epoll_->Remove(this);
			::close(fd_);
			XMA_DEBUG("%s: close fd=%d\n", Name().c_str(), fd_);
      fd_ = -1;
		}  
  }
  
  bool EpollListener::Start(int fd)
  {
    if (GetFd() != -1) {
      XMA_DEBUG("This listener is still using by fd %d", fd_);
      return false;
    }

    SetFd(fd);

    Epoll &epoll = GetContainer()->GetContext()->GetContext().GetEpoll();
    epoll.Add(this);

		return true;
  }	
  bool EpollListener::SetEvents(uint events)
  {
    XMA_DEBUG("[%s]Set events = 0x%x", Name().c_str(), events);

    if (GetFd() == -1) {
      XMA_DEBUG("%s: No associated FD.\n", Name().c_str());
      return false;
    }

    struct epoll_event ee;
    memset (&ee, 0x00, sizeof(ee));
    ee.events = events;
    ee.data.ptr = this;

    assert (epoll_ != nullptr);

    if (epoll_)
    {
      if(epoll_->Ctl(EPOLL_CTL_MOD, GetFd(), &ee) < 0)
      {
        XMA_DEBUG("(obj=%p) Failed to modify EPOLL, reason = %s", (void *)this, strerror(errno));
        return false;
      } 
      else
      {
        XMA_DEBUG("[%s] Epoll object add events done.", Name().c_str());
      }
    }
    else
    {
      XMA_DEBUG("[%s] Epoll object is null.", Name().c_str());
      return false;
    }

    
    return true;
  }

	int EpollListener::GetFd() 
	{ 
		return fd_; 
	}
	
	void EpollListener::SetFd(int fd) 
	{ 
		fd_ = fd; 
	}

	void EpollListener::SetEpoll(Epoll *epoll)
	{
		epoll_ = epoll; 
	}

///------------------------------------------------Epoll-------------------------------------------------
  Epoll::Epoll() : epoll_fd_(-1), fds_(0)
  {
    if ((epoll_fd_ = epoll_create1(EPOLL_CLOEXEC)) == -1) {
      throw std::runtime_error(std::string("Failed to create EPOLL file descriptor") + strerror(errno));
    }

    XMA_DEBUG ("EpollFD created: obj=%p, fd=%d", (void *)this, epoll_fd_);
  }

  Epoll::~Epoll()
  {
    XMA_DEBUG("EpollFD destroyed: obj=%p, fd=%d", (void *)this, epoll_fd_);
    close(epoll_fd_);
	  epoll_fd_ = -1;
  }

  void Epoll::Add(EpollListener* l)
  {
    XMA_DEBUG ("AddListener(%p): listener = %s, fd = %d", \
      (void *)this, l->Name().c_str(), l->GetFd());

    epoll_event ee;
    ee.data.ptr = l;
    //ee.events = 0;

    if (Ctl(EPOLL_CTL_ADD, l->GetFd(), &ee) < 0) {
      throw std::runtime_error(std::string("Failed to add epoll listener, err=")  + strerror(errno));		
    }
    
    l->SetEpoll(this);

    ++fds_;
  }

  void Epoll::Remove(EpollListener* l)
  {
    XMA_DEBUG ("RemoveListener(%p): listener = %s, fd = %d", \
      (void *)this, l->Name().c_str(), l->GetFd());

    if (Ctl(EPOLL_CTL_DEL, l->GetFd(), nullptr) < 0) {
      throw std::runtime_error(std::string(std::string("Failed to remove epoll listener, err=")  + strerror(errno)));		
    }

    --fds_;
  }

  int Epoll::Wait(struct epoll_event *events, int maxevents, int timeout)
  {
    return epoll_wait(GetFd(), events, maxevents, timeout);
  }
  
  int Epoll::Ctl(int op, int fd, struct epoll_event *event)
  {
    return epoll_ctl(GetFd(), op, fd, event);
  }

  int Epoll::Size() const 
  { 
    return fds_; 
  }
  
  int Epoll::GetFd(  ) const 
  { 
    return epoll_fd_;
  }
}

