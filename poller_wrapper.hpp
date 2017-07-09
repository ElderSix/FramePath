#ifndef _POLLER_WRAPPER_HPP_
#define _POLLER_WRAPPER_HPP_

#include <functional>

namespace frame_path {

enum event_type{
    EV_READ_LEVEL,
    EV_WRITE_LEVEL,
    EV_RW_LEVEL,
    //ET-Mode for epoll
    EV_READ,
    EV_WRITE,
    EV_RW,
    EV_ERR,
    EV_HUP
};

enum poller_type {
    POLLER_EPOLL,
    POLLER_NONE
};

class poller_wrapper {
public:
    virtual ~poller_wrapper() {}
    virtual int create_poller(int max_events, std::function<int(int, void*)> dispatcher) = 0;
    virtual int add_event(int fd, event_type ev_type, void *data) = 0;
    virtual int del_event(int fd) = 0;
    virtual int mod_event(int fd, event_type ev_type, void *data) = 0;
    //caller release events
    virtual int process_events(int time_wait) = 0;
};

poller_wrapper *new_poller(poller_type type, std::function<int(int, void*)> dispatcher);
void delete_poller(poller_wrapper *poller);
int add_event_to_poller(poller_wrapper *poller, int fd, event_type ev_type, void *data);
int del_event_from_poller(poller_wrapper *poller, int fd);
int mod_event_in_poller(poller_wrapper *poller, int fd, event_type ev_type, void *data);

}

#endif
