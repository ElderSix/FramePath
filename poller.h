#ifndef _POLLER_H_
#define _POLLER_H_

struct event_entry {
    event_entry():rev_handler(nullptr),wev_handler(nullptr),err_handler(nullptr),fd(0),user_data(nullptr) {}
    ~event_entry() {
        //todo
    }
    int fd;
    int type;   //event flags
    void *user_data;
    int (*rev_handler)(void *);
    int (*wev_handler)(void *);
    int (*err_handler)(void *);
};

class poller_wrapper {
public:
    poller_wrapper() = default;
    virtual ~poller_wrapper() {}
    virtual int add_poller(int fd, event_entry* ev_entry, int ev_type) = 0;
    virtual int del_poller(int) = 0;
    virtual int mod_poller(int fd, event_entry* ev_entry, int ev_type) = 0;
    //caller release events
    virtual int process_events(int time_wait) = 0;
};

#endif