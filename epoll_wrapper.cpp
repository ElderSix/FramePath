#include "epoll_wrapper.h"
#include <iostream>

using std::cin;
using std::cout;
using std::endl;

int epoll_wrapper::add_poller(int fd, event_handler* ev_handler, int ev_type) {
    //todo:ev_handler != nullptr
    if(this->epfd <= 0) {
        return -1;
    }
    int ret;
    epoll_event* ev = make_ev(fd, ev_type);
    if(!ev) {
        return -1;
    }
    ret = epoll_ctl(epfd, EPOLL_CTL_ADD, fd, ev);
    if(ret >= 0) {
        //todo:if exist?
        this->read_ev_handlers[fd] = ev_handler;
    }
    return ret;
}
int epoll_wrapper::del_poller(int fd) {
    if(this->epfd <= 0) {
        return -1;
    }
    int ret;
    //kernel > 2.6.9
    ret = epoll_ctl(epfd, EPOLL_CTL_DEL, fd, nullptr);
    return ret;
}
int epoll_wrapper::mod_poller(int fd, event_handler* ev_handler, int ev_type) {
    //todo: ev_handler != nullptr
    if(this->epfd <= 0) {
        return -1;
    }
    int ret;
    epoll_event* ev = make_ev(fd, ev_type);
    if(!ev) {
        return -1;
    }
    ret = epoll_ctl(epfd, EPOLL_CTL_MOD, fd, ev);
    if(ret >= 0) {
        //todo:if exist?
        this->write_ev_handlers[fd] = ev_handler;
    }
    return ret;
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
        }else if (event_type & EPOLLIN) {
            this->read_ev_process(fd);
        }else if (event_type & EPOLLOUT) {
            this->write_ev_handler(fd);
        }
    }
    return ret;
}

void epoll_wrapper::err_ev_process(int fd) {
    this->del_poller(fd);
    return;
}

void epoll_wrapper::read_ev_process(int fd) {
    auto iter = this->read_ev_handlers.find(fd);
    if(iter != this->read_ev_handlers.end()) {
        iter->second->handler(fd, iter->second->user_data);
    }
}

void epoll_wrapper::write_ev_handler(int fd) {
    auto iter = this->write_ev_handlers.find(fd);
    if(iter != this->write_ev_handlers.end()) {
        iter->second->handler(fd, iter->second->user_data);
    }
}














