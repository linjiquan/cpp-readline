
#pragma once

class StartServer: public Msg
{
public:
  StartServer(): Msg("StartServer") {}

  void Handler() override 
  {
    TcpEchoProcess *proc = dynamic_cast<TcpEchoProcess *>(Thread::current_);
    proc->GetService()->StartServer(address, port);
  }
  
  std::string address;
  uint16_t port;
};

class StopServer: public Msg
{
public:
  StopServer(): Msg("StopServer") {}

  void Handler() override 
  {
    TcpEchoProcess *proc = dynamic_cast<TcpEchoProcess *>(Thread::current_);
    proc->GetService()->StopServer();
  }
};

