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

using namespace std;

namespace xma {

struct Message
{
	Message(){
	}

	void Exec() {

	}

	Mailbox *sender;
	Mailbox *receiver;
};

}
