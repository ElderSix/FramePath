#include "poller.h"
#include "epoll_wrapper.h"

poller_wrapper *create_poller(int type) {
    switch(type) {
        case POLLER_EPOLL:
            return new epoll_wrapper;
        default:
            return nullptr;
    }
}

int add_event_to_poller(poller_wrapper *poller, int fd, int ev_type,
                        ev_handler rh, ev_handler wh, ev_handler eh, void *data) {
    if(!poller) {
        return -1;
    }
    int ret = poller->add_event(fd, ev_type, rh, wh, eh, data);
    return ret;
}
int del_event_from_poller(poller_wrapper *poller, int fd) {
    if(!poller) {
        return -1;
    }
    poller->del_event(fd);
    return 0;
}
int mod_event_in_poller(poller_wrapper *poller, int fd, int ev_type,
                        ev_handler rh, ev_handler wh, ev_handler eh, void *data) {
    if(!poller) {
        return -1;
    }
    int ret = poller->mod_event(fd, ev_type, rh, wh, eh, data);
    return ret;
}
