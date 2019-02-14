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
#include "../src/xma_shell.h"
#include "../src/xma_application.h"
#include "../src/xma_process.h"
#include "../src/xma_service.h"
#include "../src/xma_internal.h"
#include "../src/xma_socket.h"
#include "../src/xma_timer.h"

using namespace xma;

class TcpEchoServer;

class TcpEchoListener: public Listener
{
public:
  TcpEchoListener(TcpSocket *s): Listener(s->Name() + "/receiver", s->GetContainer())
  {
    last_uptime = 0;
  }

  bool DoHandle(void *data) override 
  {
    TcpSocket *s = reinterpret_cast<TcpSocket *>(data);

    static char buff[512];
    
    int len = s->ReadMsg(buff, 30);

    if (len < 0) { //read error
      s->Close();
      return false;
    }

    s->ShowStats();
    if (len > 0) {
      std::cout << "Recv:" << buff << std::endl;
      read_bytes+= len;
      read_pkt++;
    }

    if (last_uptime == 0) {
      last_uptime = TimeUtil::GetTime();
    }

    uint32_t curr_time = TimeUtil::GetTime();
    if (last_uptime + 3 < curr_time) {
      std::cout << "Read==> bytes: " << read_bytes << " pkts: " << read_pkt << std::endl;
      last_uptime = curr_time;
    }

    if (len > 0) {
      s->WriteMsg(buff, len);
    }

    return true;
  }
private:
  uint64_t read_bytes;
  uint64_t read_pkt;
  uint32_t last_uptime;
  bool long_conn_{false};
};


class TcpEchoServer: public TcpSocket
{
public:
  TcpEchoServer(std::string name, ListenerContainer c, uint32_t len): TcpSocket(name, c, len)
  {
    std::cout << "TCP echo server created." << std::endl;
  } 

  bool OnAccept(StreamSocket *stream_socket) override { 
    TcpSocket *s = dynamic_cast<TcpSocket *>(stream_socket);  
    s->SetReceiver(new TcpEchoListener(s));
    streams_.push_back(std::make_shared<TcpSocket>(*s));
    return true;
  }
private:
  std::vector<std::shared_ptr<TcpSocket>> streams_;
};

#if 0
class ReportStatsTimer:public Timer
{
public:
  ReportStatsTimer(std::string name, ListenerContainer c, Duration expire):Timer(name, c, expire) {}
  void Timeout() override
  {
    if (server) {
      server->ShowStats();
    }
    Set();
  }

  TcpEchoServer *server{nullptr};
};
#endif

class TcpEchoService: public Service
{
public:
  TcpEchoService(std::string address, uint16_t port) : 
    Service("TcpEchoServer") {
    address_ = address;
    port_ = port;
  }

  ~TcpEchoService() {
  }

  bool OnSocketErr(Socket *s) override
  {
    XMA_DEBUG("[%s]Get socket error. socket=%p", Name().c_str(), (void *)s);
    delete s;
    return true;
  }
  
  bool StartServer(std::string addr, uint16_t port)
  {
    if (server_ != nullptr) {
      std::cout << "Server is runing, please stop it first." << std::endl;
      return false;
    }

    address_ = addr;
    port_ = port;

    //create local tcp echo server
    server_ = new TcpEchoServer(Name(), this, 8196); 
    if (!server_->OpenServer(address_, port_, AF_INET)) {
      std::cout << "Start TCP echo server failed." << strerror(errno) << std::endl;
      delete server_;
      server_ = nullptr;
      return false;
    }
    
    std::cout << "TCP echo server is listening on : " << address_ << ":" << port_ << std::endl;
    std::cout << "Listen FD=" << server_->GetFd() << std::endl;

    return true;
  }

  bool StopServer()
  {
    if (server_ == nullptr) {
      std::cout << "Server is not runing." << std::endl;
      return false;
    }

    delete server_;
    server_ = nullptr;
    return true;
  }

  void ShowStats()
  {
    if (server_ == nullptr) {
      return ;
    }

    server_->ShowStats();
  }

  void OnInit() {    
    std::cout << "Service is runing..." << Name() << std::endl;
  }


private:
  TcpEchoServer* server_{nullptr};
  std::string address_;
  uint16_t port_;
};

