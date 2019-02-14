
// C/C++
#include <iostream>
#include <string>
#include <thread>
#include <vector>

// Linux
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <netdb.h>


// Internal
#include "server_process.h"
#include "server_service.hpp"
#include "server_msg.hpp"

TcpEchoProcess::TcpEchoProcess(): Process(TCP_SERVER_PROC, 0) 
{
  XMA_DEBUG("[%s]Process starting...", Name().c_str());
}

TcpEchoProcess::~TcpEchoProcess() 
{
  if (tcp_echo_svc_)
    delete tcp_echo_svc_;
}

TcpEchoService *TcpEchoProcess::GetService()
{
  return tcp_echo_svc_;
}

void TcpEchoProcess::OnShell(Shell &shell)
{
  shell.RegisterCommand("StartServer", "Start an echo server", [&] (ShellFuncArgs args) -> int {
    if (args.size() != 3) {
      std::cout << "Please input the server address and port" << std::endl;
      return 0;
    }

    StartServer *m = new StartServer();
    m->address = args[1];
    m->port = std::stoi(args[2]);

    if (!Msg::Send(TCP_SERVER_PROC, m))
      delete m;

    return 0;
  });

  shell.RegisterCommand("StopServer", "Stop server", [&] (ShellFuncArgs args) -> int {
    Msg::Send(TCP_SERVER_PROC, new StopServer());
    return 0;
  });

  shell.RegisterCommand("ShowStats", "Show the current server stats", [&] (ShellFuncArgs args) -> int {
    this->GetService()->ShowStats();
    return 0;
  });
}

void TcpEchoProcess::OnInit() 
{
  assert (Thread::current_ == this);

  XMA_DEBUG("[%s]Process oninit...", Name().c_str());
  tcp_echo_svc_ = new TcpEchoService("127.0.0.1", 9527);
  AddService(tcp_echo_svc_);
}


