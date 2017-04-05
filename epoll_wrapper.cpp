#include "epoll_wrapper.h"
#include <iostream>

using std::cin;
using std::cout;
using std::endl;

int epoll_wrapper::create(int n) {
    if(n <= 0) {
        return -1;
    }
    if(this->epfd > 0) {
        return this->nfds;
    }
    this->epfd = epoll_create(n);
    if(this->epfd <= 0) {
        return -1;
    }
    this->nfds = n;
    return this->nfds;
}

int epoll_wrapper::add_poller(int fd, event_entry* ev_entry, int ev_type) {
    //todo:ev_entry != nullptr
    //todo: < nfds
    if(this->epfd <= 0) {
        return -1;
    }
    if(is_fd_exist(fd)) {
        return -2;
    }
    epoll_event* ev = make_ev(fd, ev_type);
    if(!ev) {
        return -3;
    }
    int ret = epoll_ctl(epfd, EPOLL_CTL_ADD, fd, ev);
    if(ret >= 0) {
        this->ev_entries[fd] = ev_entry;
    }
    return ret;
}

int epoll_wrapper::mod_poller(int fd, event_entry* ev_entry, int ev_type) {
    if(this->epfd <= 0) {
        return -1;
    }
    if(!is_fd_exist(fd)) {
        return -2;
    }
    epoll_event* ev = make_ev(fd, ev_type);
    if(!ev) {
        return -3;
    }
    int ret = epoll_ctl(epfd, EPOLL_CTL_MOD, fd, ev);
    if(ret >= 0) {
        this->ev_entries[fd] = ev_entry;
    }
    return ret;
}

int epoll_wrapper::del_poller(int fd) {
    if(this->epfd <= 0) {
        return -1;
    }
    //kernel > 2.6.9
    if (is_fd_exist(fd)) {
        epoll_ctl(epfd, EPOLL_CTL_DEL, fd, nullptr);
        this->ev_entries.erase(fd); 
    }
    return 0;
}

int epoll_wrapper::process_events(int time_wait) {
    if(this->epfd <= 0) {
        return -1;
    }
    int n_events, fd, event_type;
    epoll_event events[this->nfds];
    n_events = epoll_wait(epfd, events, this->nfds, time_wait);
    if(n_events < 0) {
        return -1;
    }
    int ret = 0;
    for(int i = 0; i < n_events; i++) {
        fd = events[i].data.fd;
        event_type = events[i].events;
        if(event_type & (EPOLLERR|EPOLLHUP)) {
            //todo: what event will happen if client Ctrl-C?
            this->err_ev_process(fd);
            ret = fd;
        }else if(event_type & EPOLLIN) {
            this->read_ev_process(fd);
        }else if(event_type & EPOLLOUT) {
            this->write_ev_process(fd);
        }
    }
    return ret;
}

void epoll_wrapper::err_ev_process(int fd) {
    event_entry *e = get_entry_by_fd(fd);
    if(e) {
        e->err_handler(e->user_data);
        //todo: return event_entry
        this->del_poller(fd);
    }
}

void epoll_wrapper::read_ev_process(int fd) {
    event_entry *e = get_entry_by_fd(fd);
    if(e) {
        e->rev_handler(e->user_data);
    }
}

void epoll_wrapper::write_ev_process(int fd) {
    event_entry *e = get_entry_by_fd(fd);
    if(e) {
        e->wev_handler(e->user_data);
    }
}














