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

//
void Application::Show()
{
  ThreadList threads = ThreadMgr::GetThreadList();

  printf ("Thread number: %lu\n", threads.size());
  printf ("%-5s %-20s %-5s %-5s %-5s\n", "Id", "Name", "Lcore", "Tid", "State");

  for (auto &t: threads) {
    printf ("%-5u %-20s %-5u %-5u %-5u\n", t->Id(), t->Name().c_str(), t->GetRunningCore(), t->Tid(), static_cast<int>(t->GetState()));
  }
}

void Application::Exit()
{
  ThreadMgr::Exit();
}

//change the currently thread to xma thread
void Application::Init() {	
	xma::Shell &c = xma::Shell::Instance();

	static xma::Process proc_("Init", 0);

	c.RegisterCommand("ListThread", "List all threads", [](const std::vector<std::string> &) -> int {
		Show();
		return 0;
	});

	c.RegisterCommand("TestMsg", "Send a test message", [&](const std::vector<std::string> &argv) -> int {
		if (argv.size() <= 1) {
			std::cout << "Please input the send message, like TestMsg 'helloworld' " << std::endl;
			return -1;
		}

		std::cout << "Execute cmd: TestMsg " << argv[1] << std::endl;
		
		Msg *msg = new Msg(argv[1]);
		proc_.SendMsg(msg);

		return 0;
	});
}

void Application::Run() {
	xma::Shell &c = xma::Shell::Instance();
	
	// Here we call one of the defaults command of the shell, "help". It lists
	// all currently registered commands within the shell, so that the user
	// can know which commands are available.
	c.ExecCommand("help");
	
	c.Run();

	Exit();
}

}


