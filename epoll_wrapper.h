#ifndef _EPOLL_WRAPPER_H_
#define _EPOLL_WRAPPER_H_

#include "poller.h"

#include <sys/epoll.h>
#include <unistd.h>
#include <string>
#include <map>

class epoll_wrapper : public poller_wrapper {
public:
    epoll_wrapper():epfd(-1),nfds(0),name("epoll") {}
    virtual ~epoll_wrapper();
    virtual int create_poller(int max_events, ev_dispatcher dispatcher);
    virtual int add_event(int fd, int ev_type, void *data);
    virtual int del_event(int fd);
    virtual int mod_event(int fd, int ev_type, void *data);
    //caller release events
    virtual int process_events(int time_wait);
private:
    epoll_event* get_event_entry_by_fd(int fd);
    int make_epoll_ev(epoll_event** ev, int ev_type, void *data);
    int epfd;
    int nfds;
    std::string name;
    std::map<int, epoll_event*> event_group;
    ev_dispatcher dispatcher;
};

#endif
