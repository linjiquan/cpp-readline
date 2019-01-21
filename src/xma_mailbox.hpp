#pragma once
#include <thread>
#include <vector>
#include <map>
#include <iostream>
#include <mutex>
#include <atomic>
#include <assert.h>
#include "xma_status.h"

using namespace std;

namespace xma {


class Mailbox
{
public:
	Mailbox(): _epll_fd(-1) {}
	~Mailbox() = default;
	
private:
	int _epll_fd;
};
}
