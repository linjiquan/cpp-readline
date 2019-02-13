// C/C++
#include <iostream>
#include <vector>



// Linux/POSIX
#include <sys/time.h>
#include <unistd.h>


// Internal
#include "xma_timer.h"
#include "xma_process.h"

namespace xma {

///------------------Time util----------------------
uint32_t TimeUtil::GetTime()
{
  return (uint32_t)time(nullptr);
}


uint64_t TimeUtil::GetMsecs()
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return (uint64_t)(tv.tv_sec)*1000 + tv.tv_usec/1000;
}
std::string TimeUtil::TimeToStr(Timepoint t)
{
	auto time = Clock::to_time_t(t);
	struct tm ltm;
	localtime_r(&time, &ltm);

	char tstr[13]; // "22:05:43.313"
	strftime(tstr, sizeof(tstr), "%H:%M:%S", &ltm);

	auto milliseconds = std::chrono::duration_cast<Milliseconds>( t.time_since_epoch() ) - std::chrono::duration_cast<Seconds>(t.time_since_epoch());

	sprintf(tstr + 8, ".%03ld", milliseconds.count());

	return std::string(tstr);
}
///-----------------Timer----------------------------
Timer::Timer(std::string name, ListenerContainer c, Duration expire):Listener(name, c) 
{
	status_ = Status::Deactived;
	expire_ = expire;
	id_ = c->GetContext()->GetTimerMgr().GetId();
	XMA_DEBUG("[%s]Create timer, duration: %lu", Name().c_str(), expire_.count());
}

Timer::~Timer()
{
  if (status_ == Status::Actived)
  {
      assert (id_ > 0);
      Stop();
  }
}

bool Timer::Set()
{
  assert (status_ == Status::Deactived);
  tp_ = Clock::now() + expire_;
  //XMA_DEBUG("[%s]Set timer in %s", Name().c_str(), TimeUtil::TimeToStr(tp_).c_str());
  if (GetContainer()->GetContext()->SetTimer(this)) {
    assert (GetId() > 0);
    status_ = Status::Actived;
    return true;
  }

  return false;
}

bool Timer::Stop()
{
	assert (GetId() > 0 && status_ == Status::Actived);

	if (GetContainer()->GetContext()->StopTimer(this)) {
		status_ = Status::Deactived;
		return true;
	}

	return false;
}

bool Timer::Restart()
{
	return GetContainer()->GetContext()->RestartTimer(this);
}

bool Timer::DoHandle(void *data) 
{
	if (data != this) {
		XMA_DEBUG("Unknown timer %p", data);
		return false;
	}

	status_ = Status::Deactived;

	Timeout();

	return true;
}

///----------------Timer server-----------------------
TimerMgr::TimerMgr():id_(0)
{
}

bool TimerMgr::SetTimer(Timer *t)
{
	auto it = std::find_if(timers_.begin(), timers_.end(), 
			[t](const Timer *o) { return t->GetTimepoint() < o->GetTimepoint();  } );

	timers_.emplace(it, t);
	
	return true;
}

bool TimerMgr::StopTimer(Timer *t)
{
  if (t->GetId() == 0)
    return false;

  auto it = std::find_if(timers_.begin(), timers_.end(),
      [t](const Timer *o) { return t->GetId() == o->GetId(); });

  if (it != timers_.end()) {
    timers_.erase(it);
    XMA_DEBUG("[%s]Remove timer id=%lu", t->Name().c_str(), t->GetId());
    return true;
  }

  XMA_DEBUG("[%s]Remove timer failed, timer  id=%lu not found.", t->Name().c_str(), t->GetId());
  return false;
}

Duration TimerMgr::CheckTimers()
{
  Timepoint now = Clock::now();
  auto it = timers_.begin();
  while ((it != timers_.end()) && ((*it)->GetTimepoint() <= now)) {
    (*it)->DoHandle(*it);
    it = timers_.erase(it);
  }

  it = timers_.begin();
  if (it == timers_.end())
    return Duration(0);
  else
    return std::chrono::duration_cast<Duration>((*it)->GetTimepoint() - now);
}

} //namespace xma
