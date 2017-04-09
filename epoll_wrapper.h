#ifndef _EPOLL_WRAPPER_H_
#define _EPOLL_WRAPPER_H_

#include "poller.h"

#include <sys/epoll.h>
#include <string>
#include <map>

enum event_type{
    EV_READ,
    EV_WRITE,
    EV_RW,
    EV_READ_ET,
    EV_WRITE_ET,
    EV_RW_ET
};

class epoll_wrapper : public poller_wrapper {
public:
    epoll_wrapper():epfd(-1),nfds(0),name("epoll") {}
    ~epoll_wrapper() {
        //todo: release fd
    }
    int create(int n);
    int add_poller(int fd, event_entry* ev_entry, int ev_type);
    int mod_poller(int fd, event_entry* ev_entry, int ev_type);
    int del_poller(int);
    //caller release events
    int process_events(int time_wait);
private:
    struct epoll_event* make_ev(int fd, int ev_type) {
        struct epoll_event* ev = new struct epoll_event;
        ev->data.fd = fd;
        switch(ev_type) {
            case EV_READ:
                ev->events = EPOLLIN|EPOLLERR|EPOLLHUP;
                break;
            case EV_WRITE:
                ev->events = EPOLLOUT|EPOLLERR|EPOLLHUP;
                break;
            case EV_RW:
                ev->events = EPOLLIN|EPOLLOUT|EPOLLERR|EPOLLHUP;
                break;
            //After 2.6.17 we have EPOLLRDHUP for client closing connection
            case EV_READ_ET:
                ev->events = EPOLLIN|EPOLLET|EPOLLERR|EPOLLHUP|EPOLLRDHUP;
                break;
            case EV_WRITE_ET:
                ev->events = EPOLLOUT|EPOLLET|EPOLLERR|EPOLLHUP|EPOLLRDHUP;
                break;
            case EV_RW_ET:
                ev->events = EPOLLIN|EPOLLOUT|EPOLLET|EPOLLERR|EPOLLHUP|EPOLLRDHUP;
                break;
            default:
                delete ev;
                ev = nullptr;
                break;
        }
        return ev;
    }
    bool is_fd_exist(int fd) {
        return this->ev_entries.find(fd) != this->ev_entries.end();
    }
    event_entry* get_entry_by_fd(int fd) {
        auto iter = this->ev_entries.find(fd);
        if(iter != this->ev_entries.end()){
            return iter->second;
        }
        return nullptr;
    }
    void err_ev_process(int fd);
    void read_ev_process(int fd);
    void write_ev_process(int fd);
    int epfd;
    int nfds;
    std::string name;
    std::map<int, event_entry*> ev_entries;
};

#endif  
