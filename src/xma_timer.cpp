// C/C++
#include <iostream>
#include <vector>



// Linux/POSIX
#include <sys/time.h>
#include <unistd.h>


// Internal
#include "xma_timer.h"

namespace xma {

///------------------Timer helper----------------------
uint32_t Timer::GetTime()
{
  return (uint32_t)time(nullptr);
}


uint64_t Timer::GetMsecs()
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return (uint64_t)(tv.tv_sec)*1000 + tv.tv_usec/1000;
}


} //namespace xma
