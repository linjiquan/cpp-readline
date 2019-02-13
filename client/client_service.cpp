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
#include "client_service.h"

using namespace xma;

///--------------------TCP client timer--------------------------------------------------
TcpClientTimer::TcpClientTimer(std::string name, ListenerContainer c, Duration expire):Timer(name, c, expire) 
{}

TcpClientTimer::~TcpClientTimer()
{
}

void TcpClientTimer::Timeout()
{
  assert (client_ != nullptr);

  SendData();
  Set();
  client_->ShowStats();
}

void TcpClientTimer::SendData() {
  static char buff[512];
  memset (buff, 0xaa, sizeof(buff));
  client_->WriteMsg(buff, sizeof(buff));
}

///---------------TCP client-----------------------------------------------------------
TcpClient::TcpClient(ListenerContainer c): 
TcpSocket("TcpClient", c, 8196), 
timer_("TcpClientTimer", c, Duration(3000))
{
  std::cout << "TCP client created." << std::endl;
} 

TcpClient::~TcpClient() {
}

bool TcpClient::OnConnected()
{
  StartTimer();
  return true;
}

void TcpClient::StartTimer()
{
  timer_.client_ = this;
  timer_.Set();
}

///---------------TCP client service--------------------------------------------------
TcpClientService::TcpClientService() : Service("TcpClientService") 
{
}

	
TcpClientService::~TcpClientService() {

  if (client_ != nullptr)
    delete client_;
}

bool TcpClientService::OnSocketErr(Socket *s)
{
  XMA_DEBUG("[%s]Get socket error. socket=%p", Name().c_str(), (void *)s);
  if (s != client_)
    return false;

  delete s;
  client_ = nullptr;
  return true;
}

void TcpClientService::StartClient(std::string server_addr, uint16_t server_port)
{
  assert (client_ != nullptr);
  if (!client_->OpenClient(server_addr, server_port, AF_INET)) {
    std::cout << "Open client failed." << std::endl;
    return ;
  }
}

void TcpClientService::OnInit() {
  //create local tcp echo server
  client_ = new TcpClient(this); 

  //only for test
  //StartClient("127.0.0.1", 9527);
}

