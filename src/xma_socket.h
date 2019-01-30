#pragma once


// C/C++
#include <cstring>
#include <cerrno>               // std::errno()
#include <string>               // to_string()
#include <memory>
#include <iostream>

// Linux/Posix
#include <sys/socket.h>         // socket(), SOCK_STREAM, listen(), connect(), bind(), accept()
#include <netinet/tcp.h>        // TCP_NODELAY
#include <sys/un.h>             // sockaddr_un
#include <sys/stat.h>           // mkdir()
#include <unistd.h>             // close(), unlink()

// Internal
#include "xma_listener.h"
#include "xma_epoll.h"
#include "xma_service.h"
#include "xma_socket_cache.h"
#include "xma_timer.h"
namespace xma {

class StreamSocket;

//Copied /rom PNG 
#define RAW_BUFLEN(x)      (4 * (x))       // 4M
#define STREAM_RWBUFLEN(x) ((x) * 4 + 256) // 4M + 256
#define MAX_STREAM_READ    (10 << 20)      //10M

struct SocketStats
{
	uint64_t rx_pkts;  
	uint64_t tx_pkts;
	uint64_t rx_bytes;  
	uint64_t tx_bytes;  
	uint64_t rx_pkts_err;  
	uint64_t tx_pkts_err;
	uint64_t rx_bytes_err;  
	uint64_t tx_bytes_err;  
	uint64_t tx_err;  
	uint64_t rx_err;
	uint64_t tx_partial; //it is used for measure the network quality
	uint64_t tx_eagin; //it is used for measure the network quality
	uint64_t tx_cache; //it is used for measure the network quality
	uint64_t tx_cache_bytes; //it is used for measure the network quality
	uint64_t tx_direct;
  uint64_t buff_full;
  uint64_t accepted;
  uint64_t closed_by_peer;
  uint64_t rx_cache;  
  uint64_t rx_cache_bytes;
  uint64_t rx_direct;  
  uint64_t rx_dropped;
  uint64_t rx_dropped_bytes;
  uint64_t rx_oops;       // times for read to direct first, then move to cache
  uint64_t rx_oops_bytes; // bytes for read to direct first, then move to cache
};

class Socket : public EpollListener
{
public:
	Socket(std::string name, ListenerContainer c): EpollListener(name, c) {
    SetState(State::CREATED);
    SetErr(Error::NO_ERR);
    create_timestamp_ = Timer::GetTime();
    ResetStats();
  }
	virtual ~Socket() {}

  
	bool DoHandle(void * data) override;

	/// Send a message with length, the message may be cached internal
	/// On success, it return the length that sent
	/// On error, <0 is returned, and errno is set appropriately.
	///       The last err desc can be retrieved by GetErr()
	virtual int WriteMsg(const char* msg, uint32_t len) = 0;

  /// Read a completed message with the len
	/// On success, it return the length that read
	/// On error: 0 means data is not ready, app can read it later
	///           <0 means error, and errno is set appropriately.
	virtual int ReadMsg(char* msg, uint32_t len) = 0;
  
	virtual bool OpenClient(const std::string& peer_addr, uint16_t peer_port, int af) = 0;
	virtual bool OpenServer(const std::string& addr, uint16_t port, int af) = 0;


  ///wired anytime when error occours
  ///it is better to be override in the application level
  ///like change the application status and something like this
  virtual bool OnError()  {return true;}

  ///wired when new connection reach
  virtual bool OnAccept(StreamSocket *stream_socket) {return true;}
  
  // wired after connect success
  virtual bool OnConnected() { return true; }

  ///wired when FD readable
  ///Currently, I prefer to override it in the application level
  ///Maybe later, we can define a common msg hdr, then I can provider 
  ///a common read API here
  virtual bool OnRead() { throw std::runtime_error("not implemented yet");}

  ///wired when FD writable
  ///application don't override it
  ///for the package socket, it can be overridded in application level
  ///it is not tested till now
  virtual bool OnWrite() { return true;}

protected:
  virtual bool Accept() = 0;
  virtual bool Write()   = 0;
  virtual bool Connected() { return true; }
  virtual bool Read() {
   throw std::runtime_error("not supported yet");    
  }
  

	enum class State
	{
    CREATED, LISTENING, CONNECTING, CONNECTED, FULL, FAILED
	};

  enum class Error
	{
    NO_ERR, SYS_ERR, PARAM_ERR, NO_SPACE, PEER_CLOSED, READ_FAIL, 
    WRITE_FAIL, STATE_ERR, NOT_READY, UNKNOWN_EVENT, 
	};

	State GetState() const { return state_; }
	void SetState( State state ) { state_ = state; }
  std::string GetStrState() const;

  void SetErr(Error err) { last_err_ = err;}
  Error GetErr() const { return last_err_; }
  std::string GetStrErr() const;

	uint16_t GetPort() { return port_; }
	uint16_t GetPeerPort() { return peer_port_; }
	const std::string & GetAddr() { return addr_; }
	const std::string & GetPeerAddr() { return peer_addr_; }
	
	void SetPort(uint16_t port) { port_ = port; }
	void SetPeerPort(uint16_t port) { peer_port_ = port; }
	void SetAddr(const std::string & addr) { addr_ = addr; }
	void SetPeerAddr(const std::string & addr) { peer_addr_ = addr; }

  void ResetStats() { memset(&stats_, 0x00, sizeof(stats_)); }

  const char * StateToString();

public:
  SocketStats stats_;

private:
  bool DoConnect(uint events);
  bool DoReadWrite(uint events);
  bool DoListen(uint events);
  
private:
	uint16_t port_;   
	uint16_t peer_port_; 
	std::string addr_;              
	std::string peer_addr_;        
	State state_;    
  Error last_err_;
  uint32_t create_timestamp_;
};


// A C++ class wrapper for stream socket
// Each stream socket owned two caches: 
// tx_cache for writing data
// rx_cache for reading data
class StreamSocket: public Socket {
public:
  StreamSocket(std::string name, ListenerContainer c, uint32_t len);
  ~StreamSocket();

  void SetRxSize(uint32_t size);
  void SetTxSize(uint32_t size);

  /// in the future, we can use the message cache, not buffer cache
  /// it should be implemented in the application layer
  /// as only application know how to handle the retry action
	int WriteMsg(const char* msg, uint32_t len) override;
	int ReadMsg(char* msg, uint32_t len) override;
  bool Write() override;
  //bool Read() override;

  void Reset();
	void Init();
    
private:  
  int DoRead(char *buff, uint32_t len);    // the real read
  int DoWrite();

  SocketBuff rx_buff;
  SocketBuff tx_buff;
};

class TcpSocket: public StreamSocket
{
public:
	TcpSocket(std::string name, ListenerContainer c, uint32_t len): StreamSocket(name, c, len) {}
	virtual ~TcpSocket() {}

	virtual bool OpenClient(const std::string & peer_addr, uint16_t peer_port, int af) override;
	virtual bool OpenServer(const std::string & addr, uint16_t port, int af) override;
  virtual bool Connected() override;
private:
  bool ParseAddrInfo(const std::string &addr, uint16_t port, int af, bool server);
};

}
