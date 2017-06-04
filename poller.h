#ifndef _POLLER_H_
#define _POLLER_H_

enum event_type{
    EV_READ,
    EV_WRITE,
    EV_RW,
    EV_READ_ET,
    EV_WRITE_ET,
    EV_RW_ET
};

enum poller_type {
    POLLER_EPOLL,
    POLLER_NONE
};

typedef int (*ev_handler)(void *);

struct event_entry {
    int event_fd;
    int event_type;
    void *user_data;
    ev_handler rev_handler;
    ev_handler wev_handler;
    ev_handler err_handler;
};

class poller_wrapper {
public:
    virtual ~poller_wrapper() {}
    virtual int create(int n) = 0;
    virtual event_entry *create_event_entry(int fd, int ev_type,
                        ev_handler rh, ev_handler wh, ev_handler eh, void *data) = 0;
    virtual event_entry *get_event_entry_by_fd(int fd) = 0;
    virtual int add_event(int fd, int ev_type,
                            ev_handler rh, ev_handler wh, ev_handler eh, void *data) = 0;
    virtual int del_event(int fd) = 0;
    virtual int mod_event(int fd, int ev_type,
                            ev_handler rh, ev_handler wh, ev_handler eh, void *data) = 0;
    //caller release events
    virtual int process_events(int time_wait) = 0;
};

poller_wrapper *create_poller(int type);
int add_event_to_poller(poller_wrapper *poller, int fd, int ev_type,
                        ev_handler rh, ev_handler wh, ev_handler eh, void *data);
int del_event_from_poller(poller_wrapper *poller, int fd);
int mod_event_in_poller(poller_wrapper *poller, int fd, int ev_type,
                        ev_handler rh, ev_handler wh, ev_handler eh, void *data);

#endif
