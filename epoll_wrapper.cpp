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

int epoll_wrapper::create_poller(int max_events, ev_dispatcher dispatcher){
    if(max_events <= 0) {
        return -1;
    }
    if(this->epfd > 0) {
        this->dispatcher = dispatcher;
        return this->nfds;
    }
    this->epfd = epoll_create(max_events);
    if(this->epfd <= 0) {
        return -1;
    }
    this->dispatcher = dispatcher;
    this->nfds = max_events;
    return this->nfds;
}

int epoll_wrapper::make_epoll_ev(epoll_event** ev, int ev_type, void *data) {
    if(!(*ev)) {
        *ev = new struct epoll_event;
    }
    (*ev)->data.ptr = data;
    switch(ev_type) {
        case EV_READ:
            (*ev)->events = EPOLLIN|EPOLLERR|EPOLLHUP;
            break;
        case EV_WRITE:
            (*ev)->events = EPOLLOUT|EPOLLERR|EPOLLHUP;
            break;
        case EV_RW:
            (*ev)->events = EPOLLIN|EPOLLOUT|EPOLLERR|EPOLLHUP;
            break;
        //After 2.6.17 we have EPOLLRDHUP for client closing connection
        case EV_READ_ET:
            (*ev)->events = EPOLLIN|EPOLLET|EPOLLERR|EPOLLHUP|EPOLLRDHUP;
            break;
        case EV_WRITE_ET:
            (*ev)->events = EPOLLOUT|EPOLLET|EPOLLERR|EPOLLHUP|EPOLLRDHUP;
            break;
        case EV_RW_ET:
            (*ev)->events = EPOLLIN|EPOLLOUT|EPOLLET|EPOLLERR|EPOLLHUP|EPOLLRDHUP;
            break;
        default:
            return -1;
    }
    return 0;
}

epoll_event* epoll_wrapper::get_event_entry_by_fd(int fd) {
    if(event_group.find(fd) != event_group.end()) {
        return (epoll_event*)(event_group[fd]);
    }
    return nullptr;
}

int epoll_wrapper::add_event(int fd, int ev_type, void *data) {
    if(this->epfd <= 0) {
        return -1;
    }
    //todo:event_group[fd] should be nullptr
    epoll_event* ev = nullptr;
    int ret = make_epoll_ev(&ev, ev_type, data);
    if(ret != 0) {
        delete ev;
        return -3;
    }
    ret = epoll_ctl(epfd, EPOLL_CTL_ADD, fd, ev);
    if(ret != 0) {
        delete ev;
        return ret;
    }
    event_group[fd] = ev;
    return ret;
}

int epoll_wrapper::mod_event(int fd, int ev_type, void *data) {
    if(this->epfd <= 0) {
        return -1;
    }
    epoll_event *ev = get_event_entry_by_fd(fd);
    if(!ev) {
        return -5;
    }
    int ret = make_epoll_ev(&ev, ev_type, data);
    if(ret != 0) {
        return -3;
    }

    return epoll_ctl(epfd, EPOLL_CTL_MOD, fd, event_group[fd]);
}

int epoll_wrapper::del_event(int fd) {
    if(this->epfd <= 0) {
        return -1;
    }
    epoll_event *ev = get_event_entry_by_fd(fd);
    if(!ev) {
        return 0;
    }
    event_group.erase(event_group.find(fd));
    //kernel > 2.6.9
    return epoll_ctl(epfd, EPOLL_CTL_DEL, fd, nullptr);
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
        event_type = events[i].events;
        if(event_type & EPOLLRDHUP) {
            cout<<"Client closed connection"<<endl;
            this->dispatcher(EV_HUP, events[i].data.ptr);
        }else if(event_type & EPOLLERR) {
            //todo: what event will happen if client Ctrl-C?
            cout<<"Connection poll error, fd: "<<fd<<endl;
            this->dispatcher(EV_ERR, events[i].data.ptr);
        }else {
            if(event_type & EPOLLIN) {
                this->dispatcher(EV_READ, events[i].data.ptr);
            }
            if(event_type & EPOLLOUT) {
                this->dispatcher(EV_WRITE, events[i].data.ptr);
            }
        }
    }
    return ret;
}
