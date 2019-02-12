#pragma once

// Internal
#include "xma_listener.h"
#include "xma_service.h"

namespace xma {

class Epoll;

/// An C++ wrapper for fd that can be added to Epoll object
class EpollListener: public Listener {
public:
	EpollListener(std::string name, ListenerContainer c, int fd);
	EpollListener(std::string name, ListenerContainer c);
	
	virtual ~EpollListener();
	
	/// Add one or more events to listen for
	/// @param events   An EPOLL event or event bitmask
	bool SetEvents(uint events);

	int GetFd();
	void SetFd(int fd);

  // in order to reuse an EpollListener for reconnection
  void Close();
  bool Start(int fd);
	void SetEpoll(Epoll *);
  Epoll *GetEpoll() { return epoll_; }
private: 
  int fd_;
	Epoll *epoll_;
};	

/// An C++ wrapper for epoll
class Epoll
{
public:
  Epoll();
  virtual ~Epoll();
  
  /// Add an object to this file descriptor
  /// @param obj Pointer to an EpollReceiverIf derived object
  void Add(EpollListener* l);
  
  /// Delete from EPOLL file descriptor
  /// @param obj Pointer to an EpollReceiverIf derived object
  void Remove(EpollListener* l);
  
  /// Wrapper for the epoll_wait
  int Wait(struct epoll_event *events, int maxevents, int timeout);
  
  /// Wrapper for the epoll_ctl
  int Ctl(int op, int fd, struct epoll_event *event);
  
  /// Get number of associated file descriptors
  /// @returns Number of file descriptors
  int Size() const;
  
  /// Get the EPOLL file descriptor
  /// @returns An EPOLL file descriptor
  int GetFd() const;
private:
  int epoll_fd_;                          ///< Epoll file descriptor
  int fds_;                           ///< Number of file descriptors associated with this EPOLL file descriptor
};
}
