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

namespace xma {

class Socket : public EpollListener
{
public:
	Socket(std::string name);
	virtual ~Socket();

	virtual int Write(char* buff, uint32_t len) = 0;
	virtual int Read(char* buff, uint32_t len) = 0;
	virtual void StartClient(const std::string& peer_addr, uint16_t peer_port, int af) = 0;
	virtual void StartServer(const std::string& addr, uint16_t port, int af) = 0;

	enum class State
	{
	CREATED, BOUND, LISTENING, CONNECTING, CONNECTED, PEERHUP, FAILED
	};

	State GetState() const { return state_; }
	void SetState( State state ) { state_ = state; }

	uint16_t GetPort() { return port_; }
	uint16_t GetPeerPort() { return peer_port_; }
	const std::string & GetAddr() { return addr_; }
	const std::string & GetPeerAddr() { return peer_addr_; }
	
	void SetPort(uint16_t port) { port_ = port; }
	void SetPeerPort(uint16_t port) { peer_port_ = port; }
	void SetAddr(const std::string & addr) { addr_ = addr; }
	void SetPeerAddr(const std::string & addr) { peer_addr_ = addr; }

	
private:
	uint16_t port_;   
	uint16_t peer_port_; 
	std::string addr_;              
	std::string peer_addr_;        
	State state_;    
};


class TcpSocket: public Socket
{
public:
	TcpSocket(std::string name);
	virtual ~TcpSocket();

	virtual void StartClient(const std::string & peer_addr, uint16_t peer_port, int af) override;
	virtual void StartServer(const std::string & addr, uint16_t port, int af) override;
	bool DoHandle(void * data) override;
};

}
