#pragma once
#include <thread>
#include <vector>
#include <map>
#include <iostream>
#include <mutex>
#include <atomic>
#include <assert.h>
#include "xma_status.h"
#include "xma_mailbox.hpp"
#include "boost/lockfree/spsc_queue.hpp"

using namespace std;

namespace xma {

class Thread;
class Process;
class Worker;

class Task
{
public:
    Task() = default;
    ~Task() = default;
};

class ThreadContext
{
public:
	ThreadContext() = default;
	~ThreadContext() = default;
    
    using EventQueue = boost::lockfree::spsc_queue<std::shared_ptr<Task>>;

private:
	Mailbox _mailbox;
    
    vector<unique_ptr<EventQueue>> _event_queues;
};


class Thread
{
public:
	Thread(string name, uint64_t cpu_set){
		_name = name;
		_cpu_set = cpu_set;
		_id = _thread_seq++;

		_thread = std::thread(&Thread::Run, this);
	}

    virtual ~Thread() {}

	virtual void Init() {};
	virtual void Main() = 0;

	void Run()
	{
		Init();

		while (true) {
			Main();
		}
	}

	string &Name()
	{
		return _name;
	}

	u_int64_t &CpuSet()
	{
		return _cpu_set;
	}

	int32_t &Id()
	{
		return _id;
	}

private:
	static atomic_int _thread_seq;

	int32_t _id;
	string _name;
	uint64_t _cpu_set;
	thread _thread;
	ThreadContext _context;
};

class Process: public Thread
{
public:
	Process(string name, uint64_t cpu_set): Thread(name, cpu_set) {}

	virtual	void Init() {
		cout << "Process initializing: " << Name() << endl;
	}

	virtual void Main() {
		cout << "Process running: " << Name() << endl;
		this_thread::sleep_for(std::chrono::seconds(2));
	}
};


class Worker: public Thread
{
public:
	Worker(string name, uint64_t cpu_set): Thread(name, cpu_set) {}

	virtual void Init()
	{
		cout << "Worker initializing:" << Name() << endl;
	}

	virtual void Main() 
	{
		cout << "Worker running: " << Name() << endl;
		this_thread::sleep_for(std::chrono::seconds(2));
	}
private:

};
}
