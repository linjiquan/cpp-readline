
// Internal
#include "../src/xma_service.h"
#include "../src/xma_internal.h"
#include "../src/xma_socket.h"
#include "../src/xma_timer.h"

using namespace xma;

class TcpClientService;
class TcpClientTimer;
class TcpClient;

class TcpClientTimer:public Timer
{
public:
  TcpClientTimer(std::string name, ListenerContainer c, Duration expire);
  ~TcpClientTimer();
  void Timeout() override;
  void SendData();

public:
  TcpClient *client_{nullptr};
};

class TcpClient: public TcpSocket
{
public:
  TcpClient(ListenerContainer c);
  ~TcpClient();

  bool OnConnected() override;
  void StartTimer();

public:  
  TcpClientTimer timer_;
};

class TcpClientService: public Service
{
public:
  TcpClientService();
  ~TcpClientService();

  bool OnSocketErr(Socket *s) override;
  void StartClient(std::string server_addr, uint16_t server_port);
	void StopClient();
	void ShowStats();
  void OnInit();

private:
  TcpClient * client_{nullptr};
};
