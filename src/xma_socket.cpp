
// C/C++
#include <cassert>
#include <cstring>

// Linux/POSIX
#include <sys/epoll.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


// Internal
#include "xma_internal.h"
#include "xma_socket.h"

namespace xma {
///-----------------------------------Socket------------------------------------------
std::string Socket::GetStrState() const  {
  XMA_CASE_STR_BIGIN(GetState());
  XMA_CASE_STR(State::CREATED);
  XMA_CASE_STR(State::FULL);
  XMA_CASE_STR(State::LISTENING);
  XMA_CASE_STR(State::CONNECTING);
  XMA_CASE_STR(State::CONNECTED);
  XMA_CASE_STR(State::FAILED);
  XMA_CASE_STR_END();
}

std::string Socket::GetStrErr() const {
  XMA_CASE_STR_BIGIN(GetErr());
  XMA_CASE_STR(Error::NO_ERR);  
  XMA_CASE_STR(Error::SYS_ERR);
  XMA_CASE_STR(Error::NO_SPACE);
  XMA_CASE_STR(Error::READ_FAIL);
  XMA_CASE_STR(Error::PARAM_ERR);
  XMA_CASE_STR(Error::WRITE_FAIL);
  XMA_CASE_STR(Error::STATE_ERR);
  XMA_CASE_STR(Error::PEER_CLOSED);
  XMA_CASE_STR(Error::NOT_READY);
  XMA_CASE_STR(Error::UNKNOWN_EVENT);
  XMA_CASE_STR_END();
}

bool Socket::DoReadWrite(uint events)
{
  if ((events & EPOLLIN) && !OnRead()) {
    Close();
    SetState(State::FAILED);
    SetErr(Error::READ_FAIL);
    return false;
  }
  
  if ((events & EPOLLOUT) && !Write()){
    Close(); 
    SetErr(Error::WRITE_FAIL);    
    SetState(State::FAILED);
    return false;
  }
  
  if (events & (~(EPOLLIN | EPOLLOUT)))  //exception
  {
    Close();
    SetState(State::FAILED);
    SetErr(Error::UNKNOWN_EVENT);
    return OnError();
  }

  return true;
}

bool Socket::DoListen(uint events)
{
  if (events & EPOLLIN)
    return Accept();

  return true;
}

bool Socket::DoConnect(uint events)
{
    if (events & (EPOLLIN | EPOLLOUT)) {
      int error = 0;
      socklen_t ilen = sizeof(error);
      int ret = getsockopt(GetFd(), SOL_SOCKET, SO_ERROR, &error, &ilen);
      if (ret < 0) {        
        Close();
        SetErr(Error::SYS_ERR);
        return OnError();
      } else if (error == EINPROGRESS) {
        return true;
      } else if (error != 0) {
        Close();
        SetState(State::FAILED);        
        SetErr(Error::SYS_ERR);
        return OnError();
      }

      SetState(State::CONNECTED);

      return Connected();
    }
  
    return true;
}

bool Socket::DoHandle(void * data)
{
	struct epoll_event *ee = (epoll_event *)data;
	
	assert (this == ee->data.ptr);


  XMA_DEBUG("[%s]Socket sm runing: %d on %s", Name().c_str(), ee->events, GetStrState().c_str());
  
  // run the socket state machine
  switch (GetState()) {
  case State::LISTENING:
    return DoListen(ee->events);  
  case State::CONNECTING:
    return DoConnect(ee->events);

	case State::FULL:
  case State::CONNECTED:
    return DoReadWrite(ee->events);

  case State::FAILED:
	case State::CREATED:
  default:
    SetErr(Error::STATE_ERR);
    return OnError();
  }
	
	return true;  
}



///-----------------------------------Stream Socket-----------------------------------
StreamSocket::StreamSocket(std::string name, ListenerContainer c, uint32_t len):
Socket(name, c), rx_buff(1, STREAM_RWBUFLEN(len) * 2), tx_buff(0.7, STREAM_RWBUFLEN(len) * 2)
{
	Init();
}

StreamSocket::~StreamSocket()
{
  Reset();
}

void StreamSocket::SetRxSize(uint32_t size)
{
  rx_buff.setNewSize(size);
}

void StreamSocket::SetTxSize(uint32_t size)
{
  tx_buff.setNewSize(size);
}

void StreamSocket::Init()
{
	SetState(State::CREATED);
	rx_buff.setIndex(0);
	tx_buff.setIndex(0);
}

void StreamSocket::Reset() 
{
  Close();
	Init();
}

bool StreamSocket::Write()
{
  // if the stream is not ready, don't cache anything
  // this should be handled in the application layer
  if (State::CONNECTED != GetState() && State::FULL != GetState()) {
    XMA_DEBUG("[%s] stream is not ready for writing, state=%s", Name().c_str(), GetStrState().c_str());
    SetErr(Error::NOT_READY);//not ready for writing
    return false;
  }
  
  if (DoWrite() >= 0)
    return OnWrite();
  else
    return false;
}

int StreamSocket::DoWrite()
{
  assert (State::CONNECTED == GetState() || State::FULL != GetState());
  
  // send the cached data
  if (tx_buff.getIndex() > 0) {
    int ret = ::send(GetFd(), tx_buff.getBuf(), tx_buff.getIndex(), MSG_DONTWAIT);
    if (ret < 0) {
      if (errno == EINTR || errno == EAGAIN) {
				stats_.tx_eagin++;
        return 0;
      } else {
				stats_.tx_err++;
				XMA_DEBUG("[%s] stream send failed, err=%s", Name().c_str(), strerror(errno));
        return ret;
      }
    } else {
			stats_.tx_bytes += ret;
      if (ret < tx_buff.getIndex()){
        tx_buff.shrinkFromFront(ret);
				stats_.tx_partial++;
				if (GetState() == State::FULL && tx_buff.checkCanOp(0)) {
					SetState(State::CONNECTED);
				}
				XMA_DEBUG("[%s] stream partial send, bytes=%u", Name().c_str(), ret);
        return ret;
      } else {
				XMA_DEBUG("[%s] stream send, bytes=%u", Name().c_str(), ret);
        // conn->m_writeIndex = 0 ;
        tx_buff.setIndex(0);
				if (GetState() == State::FULL) {
					SetState(State::CONNECTED);
				}

        if (!AddEvents(EPOLLIN | EPOLLERR | EPOLLHUP)) {
					XMA_DEBUG("[%s] stream add events failed, err=%s", Name().c_str(), strerror(errno));
					SetErr(Error::SYS_ERR);
					return -1;
        }

        return ret;
      }
    }
  }
  return 0;
}

#define DO_CACHE_MSG(_msg, _len)        \
  do {                                  \
    if(tx_buff.copyBuf((_msg),(_len))){ \
      stats_.tx_cache_bytes += (_len);  \
      stats_.tx_cache++;                \
      if (!tx_buff.checkCanOp(0)) {     \
        SetState(State::FULL);          \
        stats_.buff_full++;             \
      }                                 \
      stats_.tx_pkts++;                 \
      ret = (len);                      \
    } else {                            \
      stats_.tx_err++;                  \
      stats_.tx_bytes_err += (_len);    \
      SetErr(Error::NO_SPACE);          \
      ret =  -1;                        \
    }                                   \
  } while (0)                      


int StreamSocket::WriteMsg(const char* msg, uint32_t len) {
  int ret = -1;
	if (msg == nullptr || len == 0) {
    stats_.tx_pkts_err++;
		SetErr(Error::PARAM_ERR);
		return -1;
	}
	if (GetState() != State::CONNECTED) {    
    stats_.tx_pkts_err++;    
    stats_.tx_bytes_err += len;
		SetErr(Error::NOT_READY);
		return -1;
	}

	//always try to send out the cached data first
  if (tx_buff.getIndex() > 0 && DoWrite() < 0) {
    stats_.tx_pkts_err++;    
    stats_.tx_bytes_err += len;    
		return -1;
	}

	if (tx_buff.getIndex() > 0){ // still has data in the cache, then append the new data to cache
    DO_CACHE_MSG(msg, len);
    return ret;
  } else {
    ret = ::send(GetFd(), msg, len, MSG_DONTWAIT);
    if (ret < 0) {
      if (errno == EINTR || errno == EAGAIN) {
        DO_CACHE_MSG(msg, len);
        return ret;
      } else { //send fail
        stats_.tx_err++;
        stats_.tx_bytes_err += len;
        SetErr(Error::SYS_ERR);
        return -1;
      }
    } else {
      if (ret < (int) len) {  //没有发送完
          DO_CACHE_MSG(msg + ret, len - ret);
          if (ret > 0) {
            stats_.tx_bytes += len - ret;
          }
          return ret;
      } else {
        stats_.tx_direct++;
        stats_.tx_pkts++;
        stats_.tx_bytes += len;
        return ret;
      }
    }
  }
}


int StreamSocket::DoRead(char *buff, uint32_t len)
{
  assert (State::CONNECTED == GetState() || State::FULL != GetState());
  assert (buff && len > 0);

  int ret = 0;
  ret = ::recv(GetFd(), buff, len, MSG_DONTWAIT);
  if (ret < 0) {
    if (errno == EINTR || errno == EAGAIN) {
      return 0;
    } else {
      stats_.rx_err++;
      SetErr(Error::READ_FAIL);
      return ret;
    }
  } else if (ret == 0) { // close by foreign peer
    stats_.closed_by_peer++;
    SetErr(Error::PEER_CLOSED);
    return -1;
  } else {
    rx_buff.addIndex(ret);
    stats_.rx_bytes += ret;
    return ret;
  }
}

int StreamSocket::ReadMsg(char* msg, uint32_t len)
{
  int ret = -1;
	if (msg == nullptr || len == 0 || len > MAX_STREAM_READ) {
		SetErr(Error::PARAM_ERR);
		return -1;
	}
	if (GetState() != State::CONNECTED && GetState() != State::FULL) {
		SetErr(Error::NOT_READY);
		return -1;
	}

#if 0 
  // no cached data, read directly
  // this feature should be test
  if (rx_buff.getIndex() == 0) {
    ret = DoRead(msg, len);
    if (ret == len) {
      stats_.rx_bytes += ret;
      stats_.rx_direct++;
      stats_.rx_pkts++;
      return ret;
    } else if (ret < 0) {
      stats_.rx_bytes_err++;
      return ret;
    } else if (ret < len) {//partion rx, need to more to the cache
      stats_.rx_oops++;
      stats_.rx_oops_bytes += ret;
      int free_size = rx_buff.getFreeLen();
      if (free_size < ret) {//no enough buffer to read 
        //data lost, too bad case
        stats_.rx_err++;
        stats_.rx_bytes += ret;
        stats_.rx_dropped++;
        stats_.rx_dropped_bytes += ret;
        SetErr(Error::SYS_ERR);
        Close();
        return -1;
      }
      
      memcpy (tx_buff.getBuf() + tx_buff.getIndex(), buff, ret);
    }
  }
#endif

  
  int read_len = rx_buff.getFreeLen();
  if(read_len > 0) {
    ret = DoRead(rx_buff.getBuf() + rx_buff.getIndex(), read_len);  
    if (ret < 0) {
      stats_.rx_err++;
      return ret;
    } else if (ret > 0) {
      stats_.rx_cache++;
      stats_.rx_cache_bytes += ret;
    }
  }

  uint32_t data_len = rx_buff.getIndex();
  if (data_len >= len) { 
    memcpy (msg, rx_buff.getBuf(), len);
    
    //need to be optimized, brant, just copied from png client
    int left_len = data_len - len;
    if (left_len > 0) {
      memmove(rx_buff.getBuf(), rx_buff.getBuf() + len, left_len);
      stats_.rx_oops ++;
      stats_.rx_oops_bytes += left_len;
    }
    
    rx_buff.setIndex(left_len);
    stats_.rx_pkts++;
    return len;
  }

  return 0;
}


bool TcpSocket::OpenServer( const std::string& addr, uint16_t port, int af)
{
 XMA_DEBUG("[%s]Open server: %s:%u", Name().c_str(), addr.c_str(), port);


 if (GetState() != State::CREATED && GetState() != State::FAILED)
 {  
   XMA_DEBUG("[%s]Open server fail in err state: %s:%u", Name().c_str(), addr.c_str(), port);
   SetErr(Error::STATE_ERR);
   return false;
 }
 
 assert (GetFd() == -1);
 
 int fd = ::socket(af, SOCK_STREAM | SOCK_NONBLOCK| SOCK_CLOEXEC, IPPROTO_TCP);
 if (fd < 0) {
   XMA_DEBUG("[%s]Open server fail, socket() fail, err:%s", Name().c_str(), strerror(errno));
   SetState(State::FAILED);
   SetErr(Error::SYS_ERR);
   return false;
 }

 int enable = 1;
 if (::setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable) ) == -1) {
   XMA_DEBUG("[%s]Open server fail, setsockopt SO_REUSEADDR, err:%s", Name().c_str(), strerror(errno));
   SetState(State::FAILED);
   SetErr(Error::SYS_ERR);
   ::close(fd);
   return false;    
 }

 struct sockaddr_in servaddr;
 bzero(&servaddr, sizeof(servaddr));
 servaddr.sin_family = AF_INET;
 servaddr.sin_addr.s_addr = inet_addr(GetAddr().c_str());
 servaddr.sin_port = htons(port);

 if (::bind(fd, (struct sockaddr*)&servaddr, sizeof(servaddr)) == -1) {
   XMA_DEBUG("[%s]Open server fail, bind address failed, err:%s", Name().c_str(), strerror(errno));
   SetState(State::FAILED);
   SetErr(Error::SYS_ERR);
   ::close(fd);
   return false;    
 }

 if(::listen(GetFd(), 10) == -1)
 {
   XMA_DEBUG("[%s]Open server fail, listen failed, err:%s", Name().c_str(), strerror(errno));
   SetState(State::FAILED);
   SetErr(Error::SYS_ERR);
   ::close(fd);
   return false;       
 }

 SetState(State::LISTENING);
 AddEvents(EPOLLIN);
 
 return true;
}

bool TcpSocket::OpenClient(const std::string & peer_addr, uint16_t peer_port, int af)
{
  XMA_DEBUG("[%s]Open client: %s:%u", Name().c_str(), peer_addr.c_str(), peer_port);
  
  if (GetState() != State::CREATED && GetState() != State::FAILED)
  {  
    XMA_DEBUG("[%s]Start client fail in err state: %s:%u", Name().c_str(), peer_addr.c_str(), peer_port);
    SetErr(Error::STATE_ERR);
    return false;
  }
  
  assert (GetFd() == -1);

  int fd = ::socket(af, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_TCP );
  if (fd < 0) {
    XMA_DEBUG("[%s]Start client fail, socket() fail, err:%s", Name().c_str(), strerror(errno));
    SetState(State::FAILED);
		SetErr(Error::SYS_ERR);
		return false;
  }

  int sock_buf_size = 1024 * 1024;
  if (0 != ::setsockopt(fd, SOL_SOCKET, SO_SNDBUF, (char*) &sock_buf_size, sizeof(sock_buf_size))) {
    XMA_DEBUG("setsockopt SO_SNDBUF error:%d,%s\n", errno, strerror(errno));
		SetState(State::FAILED);
		SetErr(Error::SYS_ERR);
		::close(fd);
		return false;
  }

  int on = 1;
  if (0 != setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &on, sizeof(on))) {
    XMA_DEBUG("setsockopt TCP_NODELAY error:%d,%s\n", errno, strerror(errno));
		SetState(State::FAILED);
		SetErr(Error::SYS_ERR);
		::close(fd);
		return false;
  }

  if (0 != setsockopt(fd, IPPROTO_TCP, TCP_QUICKACK, &on, sizeof(on))) {
    XMA_DEBUG("setsockopt TCP_QUICKACK error:%d,%s\n", errno, strerror(errno));
		SetState(State::FAILED);
		SetErr(Error::SYS_ERR);
		::close(fd);
		return false;
    // printf("setsockopt error\n");
  }

  struct sockaddr_in servaddr;
  bzero(&servaddr, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = inet_addr(peer_addr.c_str());
  servaddr.sin_port = htons(peer_port);

  //使用非阻塞连接
  int iret = connect(fd, (struct sockaddr*)&servaddr, sizeof(servaddr));
  if (iret < 0) {
    if (errno != EINPROGRESS) {
      XMA_DEBUG("png_openclient connect error:[%d:%s]", errno, strerror(errno));
			SetState(State::FAILED);
			SetErr(Error::SYS_ERR);
      ::close(fd);
      return false;
    }
  }

  SetState(State::CONNECTING);
	SetFd(fd);
  if (!AddEvents(EPOLLIN | EPOLLOUT | EPOLLERR | EPOLLHUP)) {  //未连接需要判断可读可写事件
      XMA_DEBUG("[%s] add connect events failed, error:[%d:%s]",\
					Name().c_str(), errno, strerror(errno));
			SetState(State::FAILED);
			SetErr(Error::SYS_ERR);
      Reset();
      return false;
	}	

  SetPeerAddr(peer_addr);
  SetPeerPort(peer_port);
  
	return true;
}

bool TcpSocket::Connected()
{
	struct sockaddr_in sa;
  socklen_t socklen = sizeof(sa);
	int ret = ::getsockname(GetFd(), (struct sockaddr *)&sa, &socklen);
	if (ret == -1) {
		XMA_DEBUG("[%s]Connected, getsockname failed. err:%s",\
				Name().c_str(), strerror(errno));
		//return false;
	}

	SetPort(ntohs (sa.sin_port));
	SetAddr(inet_ntoa(sa.sin_addr));


  memset(&sa, 0, sizeof(sa));
  ret = ::getpeername(GetFd(), (struct sockaddr *)&sa, &socklen);
  if(ret == -1)
  {  
		XMA_DEBUG("[%s]Connected, getpeername failed. err:%s",\
				Name().c_str(), strerror(errno));
		//return false;
  }

  SetPeerAddr(inet_ntoa(sa.sin_addr));
  SetPeerPort(ntohs(sa.sin_port));

	SetState(State::CONNECTED);

	XMA_DEBUG("[%s]OnConnected: %s:%u->%s:%u", Name().c_str(),\
    GetAddr().c_str(), GetPort(), GetPeerAddr().c_str(), GetPeerPort());
	
	return OnConnected();
}

}

