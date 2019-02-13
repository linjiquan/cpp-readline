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
#include "client_process.h"
#include "client_msg.hpp"

using namespace xma;


TcpClientProcess::TcpClientProcess(): Process(TCP_CLIENT_PROC, 0), client_(nullptr) {
  XMA_DEBUG("[%s]Process starting...", Name().c_str());
}

TcpClientProcess::~TcpClientProcess() {
  if (client_ != nullptr) {
    delete client_;
  }
}

void TcpClientProcess::RegisterCommand()
{
  assert (Thread::current_ == this);

  Shell &s = Shell::Instance();

  s.RegisterCommand("Connect", "Connect to an echo server", [&] (ShellFuncArgs args) -> int {
    if (args.size() != 3) {
      std::cout << "Please input the server address and port" << std::endl;
      return 0;
    }

    StartConnect *m = new StartConnect();
    m->address = args[1];
    m->port = std::stoi(args[2]);

    if (!Msg::Send(TCP_CLIENT_PROC, m))
      delete m;

    return 0;
  });

  s.RegisterCommand("Disconnect", "Disconnect the current connection", [&] (ShellFuncArgs args) -> int {
    Msg::Send(TCP_CLIENT_PROC, new StopConnect());
    return 0;
  });

  s.RegisterCommand("ShowStats", "Show the current connection stats", [&] (ShellFuncArgs args) -> int {
    this->GetClientService()->ShowStats();
    return 0;
  });
}

TcpClientService * TcpClientProcess::GetClientService() {
  return client_;
}

void TcpClientProcess::OnInit() {

  assert (Thread::current_ == this);

  client_ = new TcpClientService();
  AddService(client_);
}
