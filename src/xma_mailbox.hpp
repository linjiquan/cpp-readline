#pragma once
#include <thread>
#include <vector>
#include <map>
#include <iostream>
#include <mutex>
#include <atomic>
#include <assert.h>
#include "xma_status.h"
#include "event.h"

using namespace std;

namespace xma {


class Mailbox
{
public:
	Mailbox(){
		_base = event_base_new();	
	}

	~Mailbox() {
		event_base_free(_base);
	}
	
	void Dispatch() {
		event_base_dispatch(_base);
	}		

private:
	event_base* _base;
	//int _epll_fd;
};
}
