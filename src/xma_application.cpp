#include <iostream>
#include <atomic>
#include <vector>
#include <thread>
#include <string>
#include <mutex>
#include <condition_variable>
#include <future>
#include "xma_application.hpp"
#include "xma_shell.hpp"

using namespace std;

namespace xma {
///------------------------------Application-------------------------------------
//Thread reflection
vector<Thread *> Application::_threads;
mutex Application::_threads_mutex;

//
void Application::Show()
{
	unique_lock<mutex> lock(_threads_mutex);

	cout << "Thread number: " << _threads.size() << endl;
	cout << "Id" << "\t" << "Name" << "\t" << "Lcore" << "\t" << "Queuesize" << endl;

	for (auto &t: _threads) {
		cout << t->Id() << "\t" << t->Name() << "\t" << t->CpuSet() << "\t" << t->QueueSize() << endl;
	}
}


//change the currently thread to xma thread
void Application::Init() {	
	xma::Shell &c = xma::Shell::Instance();

	c.RegisterCommand("ListThread", "List all threads", [](const std::vector<std::string> &) -> int {
		Show();
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
}

bool Application::Register(Thread *t)
{
	unique_lock<mutex> lock(_threads_mutex);
	_threads.push_back(t);

	return true;
}
}


