#include "../src/xma_shell.h"
#include "../src/xma_application.h"
#include "../src/xma_process.h"
#include "../src/xma_service.h"
#include "../src/xma_internal.h"
#include "../src/xma_socket.h"
#include "../src/xma_timer.h"

using namespace xma;

class TcpEchoService;

#define TCP_SERVER_PROC     "TcpServer"

class TcpEchoProcess: public Process
{
public:
  TcpEchoProcess();
  ~TcpEchoProcess();

  TcpEchoService *GetService();

  void OnShell(Shell &shell) override;
  void OnInit() override;
  
  TcpEchoService *tcp_echo_svc_{nullptr};
};

