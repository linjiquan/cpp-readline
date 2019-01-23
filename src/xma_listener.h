#pragma once

#include "event.h"

namespace xma {
class Listener {
public:
    using Handler = std::function<void()>;

    virtual ~Listener();

    bool Init();
    void Cancel();
    void SetCancelCallback(const Handler& cb);
protected:
    // @note It MUST be called in the event thread.
    // @param timeout the maximum amount of time to wait for the event, or 0 to wait forever
    bool Listen(struct timeval *timeout);

protected:
    Listener(struct event_base* evbase, const Handler& handler);
    Listener(struct event_base* evbase, Handler&& handler);

    void Close();
    void FreeEvent();

    virtual bool DoInit() = 0;
    virtual void DoClose() {}

protected:
    struct event* event_;
    struct event_base* evbase_;
    Handler handler_;
    Handler cancel_handler_;
};

class MessageListener : public Listener {
public:
    MessageListener(struct event_base* evbase, const Handler& handler);
    MessageListener(struct event_base* evbase, Handler&& handler);
    ~MessageListener();

    bool AsyncWait();
    void Notify();
    evpp_socket_t wfd() const { return pipe_[0]; }
private:
    virtual bool DoInit();
    virtual void DoClose();
    static void HandlerFn(evpp_socket_t fd, short which, void* v);

    evpp_socket_t pipe_[2]; // Write to pipe_[0] , Read from pipe_[1]
};
}

