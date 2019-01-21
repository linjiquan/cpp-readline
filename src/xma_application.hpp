#pragma once
#include <thread>
#include <vector>
#include <map>
#include <iostream>
#include <mutex>
#include <atomic>
#include <assert.h>
#include "xma_thread.hpp"

using namespace std;

namespace xma {

class Thread;
class Process;
class Worker;

using ThreadCreateFunction = std::function<Thread * (string name, uint64_t cpu_set, uint32_t number)>;

class Application
{
public:

	Application() = default;
	~Application() = default;

	static bool Register(Thread *t);

	//change the currently thread to xma thread
	static void Init();
	static void Show();
	static void Run();

private:
	static vector<Thread *> _threads;
	static mutex _threads_mutex;
};

}
