// C/C++
#include <iostream>
#include <string>
#include <thread>
#include <vector>

// Linux
#include <sys/types.h>
#include <sys/socket.h>         // SOCK_STREAM, AF_INET, AF_INET6, AF_UNSPEC, getsockname(), send((), recv(), SO_REUSEADDR, SOL_SOCKET, setsockopt(), getsockopt(), SO_ERROR
#include <netinet/in.h>         // sockaddr_in, htons(), ntohs(), IN6ADDR_ANY_INIT, SOL_IPV6, IPV6_V6ONLY
#include <netinet/ip.h>
#include <arpa/inet.h>          // inet_pton(), inet_ntoa()
#include <netdb.h>                      // AI_PASSIVE, NI_NUMERICHOST, getnameinfo(), getaddrinfo(), getprotobynumber()


// Internal
#include "../src/xma_shell.h"
#include "../src/xma_application.h"
#include "../src/xma_process.h"
#include "../src/xma_service.h"
#include "../src/xma_internal.h"
#include "../src/xma_socket.h"

using namespace xma;

class TcpEchoServer;

class TcpEchoSocket: public TcpSocket
{
public:
	TcpEchoSocket(std::string name, ListenerContainer c, uint32_t len): TcpSocket(name, c, len)
  {
    std::cout << "TCP echo server created." << std::endl;
  } 

  
  bool OnAccept(StreamSocket *stream_socket) override { 
    //Service *server = dynamic_cast<Service *>(GetContainer());
    
    //server->OnAccept(stream_socket);
		if (stream_socket == nullptr)
			return false;

    return true;
  }
private:
};


class TcpEchoServer: public Service
{
public:
	TcpEchoServer(std::string address, uint16_t port) : Service("TcpEchoServer"){
		address_ = address;
		port_ = port;
	}

  ~TcpEchoServer() {
    if (server_ != nullptr)
      delete server_;
  }

  
	void OnInit() {
		//create local tcp echo server
		server_ = new TcpEchoSocket(Name(), this, 8196); 
    if (!server_->OpenServer(address_, port_, AF_INET)) {
      throw std::runtime_error("Start TCP echo server failed.");
    }
    
		std::cout << "TCP echo server is listening on : " << address_ << ":" << port_ << std::endl;
    std::cout << "Listen FD=" << server_->GetFd() << std::endl;
	}

private:
	TcpEchoSocket *server_;
	std::string address_;
	uint16_t port_;
};

class TcpEchoProcess: public Process
{
public:
  TcpEchoProcess(): Process("TCP-Echo", 0), server("127.0.0.1", 9527) {
  }

  void OnInit() override {
		AddService(&server);
	}
  
  TcpEchoServer server;
};

TcpEchoProcess tcp_echo_server;

