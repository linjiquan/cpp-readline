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
#include <netdb.h>              // AI_PASSIVE, NI_NUMERICHOST, getnameinfo(), getaddrinfo(), getprotobynumber()


// Internal
#include "../src/xma_shell.h"
#include "../src/xma_application.h"
#include "../src/xma_process.h"
#include "../src/xma_service.h"
#include "../src/xma_internal.h"
#include "../src/xma_socket.h"
#include "../src/xma_timer.h"

using namespace xma;

class TcpClientService;

class TcpClient: public TcpSocket
{
public:
  TcpClient(ListenerContainer c): TcpSocket("TcpClient", c, 8196)
  {
    std::cout << "TCP client created." << std::endl;
  } 

};

class TcpClientTimer:public Timer
{
public:
  TcpClientTimer(std::string name, ListenerContainer c, Duration expire):Timer(name, c, expire) {}
  void Timeout() override
  {
    SendData();
		Set();
    client_->ShowStats();
  }
  
  void SendData() {
    static char buff[512];
    memset (buff, 0xaa, sizeof(buff));
    client_->WriteMsg(buff, sizeof(buff));
  }
  
  TcpClient *client_;
};


class TcpClientService: public Service
{
public:
	TcpClientService() : Service("TcpClientService") {
	}

	
	~TcpClientService() {
    if (client_ != nullptr)
      delete client_;
	}

  void StartClient(std::string server_addr, uint16_t server_port)
  {
    client_->OpenClient(server_addr, server_port, AF_INET);
    tcp_client_timer_ = std::unique_ptr<TcpClientTimer>(new TcpClientTimer("TcpClientTimer", this, Duration(3000)));
    tcp_client_timer_->client_ = client_;
    tcp_client_timer_->Set();
  }
  
	void OnInit() {
 		//create local tcp echo server
		client_ = new TcpClient(this); 
    
	}


  
private:
  std::unique_ptr<TcpClientTimer> tcp_client_timer_;
  TcpClient * client_;
};
