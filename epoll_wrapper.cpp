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

struct epoll_event* epoll_wrapper::make_epoll_ev(event_entry* ee) {
    struct epoll_event* ev = new struct epoll_event;
    bool is_valid_entry = false;
    ev->data.fd = ee->event_fd;
    ev->data.ptr = ee;
    switch(ee->event_type) {
        case EV_READ:
            ev->events = EPOLLIN|EPOLLERR|EPOLLHUP;
            is_valid_entry = ee->rev_handler;
            break;
        case EV_WRITE:
            ev->events = EPOLLOUT|EPOLLERR|EPOLLHUP;
            is_valid_entry = ee->wev_handler;
            break;
        case EV_RW:
            ev->events = EPOLLIN|EPOLLOUT|EPOLLERR|EPOLLHUP;
            is_valid_entry = (ee->rev_handler) && (ee->wev_handler);
            break;
        //After 2.6.17 we have EPOLLRDHUP for client closing connection
        case EV_READ_ET:
            ev->events = EPOLLIN|EPOLLET|EPOLLERR|EPOLLHUP|EPOLLRDHUP;
            is_valid_entry = ee->rev_handler;
            break;
        case EV_WRITE_ET:
            ev->events = EPOLLOUT|EPOLLET|EPOLLERR|EPOLLHUP|EPOLLRDHUP;
            is_valid_entry = ee->wev_handler;
            break;
        case EV_RW_ET:
            ev->events = EPOLLIN|EPOLLOUT|EPOLLET|EPOLLERR|EPOLLHUP|EPOLLRDHUP;
            is_valid_entry = (ee->rev_handler) && (ee->wev_handler);
            break;
        default:
            break;
    }
    if(!is_valid_entry) {
        delete ev;
        ev = nullptr;
    }
    return ev;
}

event_entry* epoll_wrapper::create_event_entry(int fd, int ev_type,
                    ev_handler rh, ev_handler wh, ev_handler eh, void *data) {
    event_entry *entry = new struct event_entry;
    entry->event_fd = fd;
    entry->event_type = ev_type;
    entry->user_data = data;
    entry->rev_handler = rh;
    entry->wev_handler = wh;
    entry->err_handler = eh;
    return entry;
}

event_entry* epoll_wrapper::get_event_entry_by_fd(int fd) {
    if(event_group.find(fd) != event_group.end()) {
        return (event_entry*)(event_group[fd]->data.ptr);
    }
    return nullptr;
}

int epoll_wrapper::add_event(int fd, int ev_type,
                        ev_handler rh, ev_handler wh, ev_handler eh, void *data) {
    //todo:ee != nullptr
    //todo: < nfds
    if(this->epfd <= 0) {
        return -1;
    }
    event_entry* ee = create_event_entry(fd, ev_type, rh, wh, eh, data);
    if(!ee) {
        return -3;
    }
    epoll_event* ev = make_epoll_ev(ee);
    if(!ev) {
        delete ee;
        return -3;
    }
    int ret = epoll_ctl(epfd, EPOLL_CTL_ADD, ee->event_fd, ev);
    if(ret != 0) {
        delete ee;
        delete ev;
        return ret;
    }
    event_group[ee->event_fd] = ev;
    return ret;
}

int epoll_wrapper::mod_event(int fd, int ev_type,
                        ev_handler rh, ev_handler wh, ev_handler eh, void *data) {
    if(this->epfd <= 0) {
        return -1;
    }
    event_entry *ee = get_event_entry_by_fd(fd);
    if(!ee) {
        return -5;
    }
    ee->event_type = ev_type;
    ee->user_data = data;
    ee->rev_handler = rh;
    ee->wev_handler = wh;
    ee->err_handler = eh;

    int ret = epoll_ctl(epfd, EPOLL_CTL_MOD, ee->event_fd, event_group[fd]);
    return ret;
}

int epoll_wrapper::del_event(int fd) {
    if(this->epfd <= 0) {
        return -1;
    }
    event_entry *ee = get_event_entry_by_fd(fd);
    if(!ee) {
        return 0;
    }
    //kernel > 2.6.9
    int ret = epoll_ctl(epfd, EPOLL_CTL_DEL, ee->event_fd, nullptr);
    event_group.erase(event_group.find(fd));
    delete ee;
    return ret;
}

int epoll_wrapper::process_events(int time_wait) {
    if(this->epfd <= 0) {
        return -1;
    }
    int n_events, fd, event_type;
    event_entry *ee;
    epoll_event events[this->nfds];
    n_events = epoll_wait(epfd, events, this->nfds, time_wait);
    if(n_events < 0) {
        return -1;
    }
    int ret = 0;
    for(int i = 0; i < n_events; i++) {
        fd = events[i].data.fd;
        event_type = events[i].events;
        ee = (event_entry *)events[i].data.ptr;
        if(event_type & EPOLLRDHUP) {
            cout<<"Client closed connection"<<endl;
            continue;
        }
        if(event_type & (EPOLLERR|EPOLLHUP)) {
            //todo: what event will happen if client Ctrl-C?
            cout<<"Connection poll error, fd: "<<fd<<endl;
            ee->err_handler(ee->user_data);
        }else {
            if(event_type & EPOLLIN) {
                ee->rev_handler(ee->user_data);
            }
            if(event_type & EPOLLOUT) {
                ee->wev_handler(ee->user_data);
            }
        }
    }
    return ret;
}
