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

using namespace xma;

#define TCP_CLIENT  "TcpClient"

TcpClientProcess::TcpClientProcess(): Process(TCP_CLIENT, 0), client_(nullptr) {
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

    class StartConnect: public Msg
    {
    public:
      StartConnect(): Msg("StartConnect") {}

      void Handler() override 
      {
        TcpClientProcess *proc = dynamic_cast<TcpClientProcess *>(Thread::current_);
        proc->client_->StartClient(address, port);
      }
      
      std::string address;
      uint16_t port;
    };

    StartConnect *m = new StartConnect();
    m->address = args[1];
    m->port = std::stoi(args[2]);

    if (!Msg::Send(TCP_CLIENT, m))
      delete m;

    return 0;
  });
}

void TcpClientProcess::OnInit() {

  assert (Thread::current_ == this);

  client_ = new TcpClientService();
  AddService(client_);
}
