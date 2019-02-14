// C/C++
#include <iostream>
#include <atomic>
#include <vector>
#include <thread>
#include <string>
#include <mutex>
#include <condition_variable>
#include <future>


// Linux/POSIX
#include <unistd.h>

// Internal
#include "xma_internal.h"
#include "xma_application.h"
#include "xma_shell.h"
#include "xma_process.h"

namespace xma {

void Application::Exit()
{
  ThreadMgr::Exit();
}

//change the currently thread to xma thread
//tbd
void Application::Init() {  
  xma::Shell &c = xma::Shell::Instance();

  static xma::Process ____init____("Init", 0);

  assert (Thread::current_ == nullptr);

  Thread::current_ = &____init____;

  c.RegisterCommand("ListThread", "List all threads", [](const std::vector<std::string> &) -> int {
    ThreadMgr::List();
    return 0;
  });

  c.RegisterCommand("ListService", "List all services", [](const std::vector<std::string> &) -> int {
    Service::List();
    return 0;
  });

  c.RegisterCommand("ListListener", "List all listeners", [](const std::vector<std::string> &) -> int {
    std::cout << "Version: 0.0.1" << std::endl;  
    std::cout << "Build time: " << __DATE__ << "  " << __TIME__ << std::endl;
    return 0;
  });

  c.RegisterCommand("Version", "Show version", [](const std::vector<std::string> &) -> int {
    Listener::List();
    return 0;
  });

  c.RegisterCommand("TestMsg", "Send a test message", [&](const std::vector<std::string> &argv) -> int {
    if (argv.size() <= 1) {
      std::cout << "Please input the send message, like TestMsg 'helloworld' " << std::endl;
      return -1;
    }

    std::cout << "Execute cmd: TestMsg " << argv[1] << std::endl;
    
    Msg *msg = new Msg(argv[1]);
    msg->SetFlag(XMA_MSG_DEBUG);
    if (!Msg::Send("Init", msg)) {
      delete msg;
    }

    return 0;
  });
}

void Application::Run() {
  xma::Shell &c = xma::Shell::Instance();

  c.Run();

  Exit();
}

}


