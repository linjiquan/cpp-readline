#pragma once

// C/C++
#include <thread>
#include <vector>
#include <map>
#include <iostream>
#include <mutex>
#include <atomic>
#include <assert.h>

// Internal
#include "xma_status.h"

namespace xma {
	
class Msg{
public:
	Msg(std::string sval): sval_(sval){		
	}

	~Msg() {}

	std::string &GetValue()
	{
		return sval_;
	}

private:
	uint16_t type_;
	std::string sval_;
};
}
