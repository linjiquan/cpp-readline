#pragma once

// C/C++
#include <chrono>
#include <algorithm>

// Internal
#include "xma_listener.h"

namespace xma {

class Timer;
class TimerMgr;

using Clock = std::chrono::high_resolution_clock;
using Timepoint = Clock::time_point;
using Duration = std::chrono::milliseconds;
using Microseconds = std::chrono::microseconds;
using Milliseconds = std::chrono::milliseconds;
using Seconds = std::chrono::seconds;

// A util class for time
class TimeUtil
{
public:  
    /* returns the time as the number of seconds since the Epoch, 1970-01-01 00:00:00 +0000 (UTC). */
    static uint32_t GetTime();
    static uint64_t GetMsecs();
		static std::string TimeToStr(Timepoint t);
};

class Timer: public Listener
{
public:
	Timer(std::string name, ListenerContainer c, Duration expire);
	virtual ~Timer();

	enum class Status
	{
		Actived, Deactived
	};

	bool DoHandle(void *data) override;
	
	bool Set();
	bool Stop();
	bool Restart();
	bool Restart(Duration expire);

	virtual void Timeout() = 0;

	Timepoint GetTimepoint() const { return tp_; }
	uint64_t GetId() const { return id_; }
	void SetId(uint64_t id) { id_ = id; }
private:
	Duration expire_;
	Status status_;
	uint64_t id_;
	Timepoint tp_;
};

class TimerMgr
{
public:
	TimerMgr();
	virtual ~TimerMgr() {}

	bool SetTimer(Timer *t);
	bool StopTimer(Timer *t);

	Duration CheckTimers();

	void List();
	uint64_t GetId() { return ++id_;}

private:
	uint64_t id_;

	std::list<Timer *> timers_;
};

}
