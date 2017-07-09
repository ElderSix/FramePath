#ifndef _EPOLL_WRAPPER_HPP_
#define _EPOLL_WRAPPER_HPP_

#include "poller_wrapper.hpp"

#include <sys/epoll.h>
#include <unistd.h>
#include <string>
#include <map>

namespace frame_path {

class epoll_wrapper : public poller_wrapper {
public:
    epoll_wrapper():epfd(-1),nfds(0),name("epoll"),dispatcher(nullptr) {}
    virtual ~epoll_wrapper();
    virtual int create_poller(int max_events, std::function<int(int, void*)> dispatcher);
    virtual int add_event(int fd, event_type ev_type, void *data);
    virtual int del_event(int fd);
    virtual int mod_event(int fd, event_type ev_type, void *data);
    //caller release events
    virtual int process_events(int time_wait);
private:
    epoll_event* get_event_entry_by_fd(int fd);
    int make_epoll_ev(epoll_event** ev, event_type ev_type, void *data);
    int epfd;
    int nfds;
    std::string name;
    std::map<int, epoll_event*> event_group;
    std::function<int(int, void*)> dispatcher;
};

}
#endif
