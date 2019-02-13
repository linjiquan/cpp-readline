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

#define XMA_MSG_DEBUG   0x01

class Msg{
public:

  static bool Send(std::string receiver, Msg *msg);

public:
  Msg(std::string name, std::string sval): name_(name), sval_(sval) {}
  Msg(std::string name, uint32_t val): val_(val), name_(name) {}
  Msg(std::string name): name_(name) {}
  Msg() {}

  virtual ~Msg() {}
  virtual void Handler() {}

  void SetName(const std::string &name) { name_ = name; }
  std::string &GetName() { return name_; }

  std::string &GetStrVal() { return sval_; }
  uint32_t GetVal() { return val_; }

  void SetSender(const std::string &sender) { sender_ = sender; }
  std::string &GetSender() { return sender_;}

  void SetReceiver(const std::string &receiver) { receiver_ = receiver; }
  std::string &GetReceiver() { return receiver_; }

  void SetFlag(uint32_t flag) { flag_ = flag; }
  uint32_t GetFlag() { return flag_; }

  void Exec();

private:
  uint32_t flag_;
  uint32_t val_;
  std::string name_;
  std::string sender_;
  std::string receiver_;
  std::string sval_;
};
}
