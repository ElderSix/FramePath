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
    virtual ~epoll_wrapper() {
        if(epfd >= 0) {
            close(epfd);
        }
        //TODO: Release epoll_event allocated by make_ev

    }
    virtual int create(int n);
    virtual int add_event(int fd, int ev_type,
                            ev_handler rh, ev_handler wh, ev_handler eh, void *data);
    virtual int del_event(int fd);
    virtual int mod_event(int fd, int ev_type,
                            ev_handler rh, ev_handler wh, ev_handler eh, void *data);
    //caller release events
    virtual int process_events(int time_wait);
private:
    virtual event_entry *create_event_entry(int fd, int ev_type,
                        ev_handler rh, ev_handler wh, ev_handler eh, void *data);
    virtual event_entry *get_event_entry_by_fd(int fd);
    struct epoll_event* make_epoll_ev(event_entry* ee);
    int epfd;
    int nfds;
    std::string name;
    std::map<int, epoll_event*> event_group;
};

#endif
