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
#include "xma_message.h"
#include "boost/lockfree/spsc_queue.hpp"
//#include "xma_listener.h"

using namespace std;

namespace xma {

class Thread;
class Process;
class Worker;

class ThreadContext
{
public:
	ThreadContext() = default;
	~ThreadContext() = default;

    using MsgQueue = boost::lockfree::spsc_queue<std::shared_ptr<Message>>;

	uint32_t QueueSize()
	{
		return _msg_queues.size();	
	}

	vector<unique_ptr<MsgQueue>> &GetTaskQueue()
	{
		return _msg_queues;
	}

private:
    vector<unique_ptr<MsgQueue>> _msg_queues;
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

	uint32_t QueueSize()
	{
		return _context.QueueSize();
	}

	void Run()
	{
		Init();

		while (true) {
			for (auto &queue : _context.GetTaskQueue()) {
				if (queue->empty()) continue;
				auto evt = queue->front();
				queue->pop();
				evt->Exec();
			}

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
		_mailbox.Dispatch();
	}

private:
	Mailbox _mailbox;
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
