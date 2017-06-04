#ifndef _POLLER_H_
#define _POLLER_H_

enum event_type{
    EV_READ,
    EV_WRITE,
    EV_RW,
    EV_READ_ET,
    EV_WRITE_ET,
    EV_RW_ET,
    EV_ERR,
    EV_HUP
};

enum poller_type {
    POLLER_EPOLL,
    POLLER_NONE
};

typedef int (*ev_handler)(void *);
typedef int (*ev_dispatcher)(int, void *);

class poller_wrapper {
public:
    virtual ~poller_wrapper() {}
    virtual int create(int n) = 0;
    virtual int create_poller(int max_events, ev_dispatcher dispatcher) = 0;
    virtual int add_event(int fd, int ev_type, void *data) = 0;
    virtual int del_event(int fd) = 0;
    virtual int mod_event(int fd, int ev_type, void *data) = 0;
    //caller release events
    virtual int process_events(int time_wait) = 0;
};

poller_wrapper *new_poller(int type);
int add_event_to_poller(poller_wrapper *poller, int fd, int ev_type, void *data);
int del_event_from_poller(poller_wrapper *poller, int fd);
int mod_event_in_poller(poller_wrapper *poller, int fd, int ev_type, void *data);

#endif
