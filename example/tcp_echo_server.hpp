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

class TcpEchoService: public Service
{
public:
  TcpEchoService(std::string address, uint16_t port) : 
    Service("TcpEchoServer") {
    address_ = address;
    port_ = port;
  }

  ~TcpEchoService() {
     if (stats_report_timer_) {
      delete stats_report_timer_;
     }
      
      if (server_) {
        delete server_;
      }
  }

  bool OnSocketErr(Socket *s) override
  {
    XMA_DEBUG("[%s]Get socket error. socket=%p", Name().c_str(), (void *)s);
    delete s;
    return true;
  }
  
  void OnInit() {    
    //create local tcp echo server
    server_ = new TcpEchoServer(Name(), this, 8196); 
    if (!server_->OpenServer(address_, port_, AF_INET)) {
      throw std::runtime_error("Start TCP echo server failed.");
    }
    
    std::cout << "TCP echo server is listening on : " << address_ << ":" << port_ << std::endl;
    std::cout << "Listen FD=" << server_->GetFd() << std::endl;

    stats_report_timer_ = new ReportStatsTimer("TcpEchoServerReportTimer", this, Duration(3000));
    stats_report_timer_->server = server_;
    if (stats_report_timer_->Set()) {
      std::cout << "TCP echo server report timer is started..." << std::endl;
    } else {
      std::cout << "TCP echo server report timer started failed." << std::endl;
    }
  }


private:
  TcpEchoServer* server_{nullptr};
  std::string address_;
  uint16_t port_;
  ReportStatsTimer *stats_report_timer_{nullptr};
};

class TcpEchoProcess: public Process
{
public:
  TcpEchoProcess(): Process("TCP-Echo", 0) {
    XMA_DEBUG("[%s]Process starting...", Name().c_str());
  }

  ~TcpEchoProcess() {
    if (tcp_echo_svc_)
      delete tcp_echo_svc_;
  }

  void RegisterCommand() override
  {
    Shell &s = Shell::Instance();
    s.RegisterCommand("StartEchoServer", "Start an echo server", [&] (ShellFuncArgs args) -> int {
      std::cout << "not implemented yet." << std::endl;
      return 0;
    });
  }

  void OnInit() override {
    assert (Thread::current_ == this);

    XMA_DEBUG("[%s]Process oninit...", Name().c_str());
    tcp_echo_svc_ = new TcpEchoService("127.0.0.1", 9527);
    AddService(tcp_echo_svc_);
  }
  
  TcpEchoService *tcp_echo_svc_;
};

