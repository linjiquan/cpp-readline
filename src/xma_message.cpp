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
#include "xma_thread.h"
#include "xma_message.h"

using namespace xma;

bool Msg::Send(std::string receiver, Msg *msg)
{
  assert (Thread::current_ != nullptr);
  Thread *r = ThreadMgr::GetThread(receiver);
  if (r == nullptr) {
    std::cout << "Invalid receiver: " << receiver << std::endl;
    return false;
  }

  msg->SetReceiver(receiver);
  msg->SetSender(Thread::current_->Name());

  return r->SendMsg(msg);
}

void Msg::Exec() 
{
  if (GetFlag() & XMA_MSG_DEBUG) {
    std::cout << GetName() << "  :  " << GetSender() << "    ->    " << GetReceiver() << std::endl;
  }

  Handler();
}
