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
#include "client_service.hpp"

using namespace xma;

class TcpClientProcess: public Process
{
public:
  TcpClientProcess(): Process("TCP-Client", 0), client_(nullptr) {
    XMA_DEBUG("[%s]Process starting...", Name().c_str());
  }

  ~TcpClientProcess() {
	  if (client_ != nullptr) {
		  delete client_;
	  }
  }

  void RegisterCommand() override
  {
    Shell &s = Shell::Instance();

    
    s.RegisterCommand("Connect", "Connect to an echo server", [&] (ShellFuncArgs args) -> int {
      if (args.size() != 3) {
        std::cout << "Please input the server address and port" << std::endl;
        return 0;
      }

      client_->StartClient(args[1], std::stoi(args[2]));

      return 0;
    });
  }

  void OnInit() override {
	  client_ = new TcpClientService();
	  AddService(client_);
  }

private:
  TcpClientService *client_;
};
