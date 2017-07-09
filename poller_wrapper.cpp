#include "poller_wrapper.hpp"
#include "epoll_wrapper.hpp"

using namespace frame_path;

poller_wrapper *frame_path::new_poller(poller_type type, std::function<int(int, void*)> dispatcher) {
    switch(type) {
        case POLLER_EPOLL:
            {
                epoll_wrapper *ret = new epoll_wrapper;
                if(ret) {
                    ret->create_poller(10, dispatcher);
                }
                return ret;
            }
        default:
            return nullptr;
    }
}

void frame_path::delete_poller(poller_wrapper *poller) {
    delete poller;
}

int frame_path::add_event_to_poller(poller_wrapper *poller, int fd, event_type ev_type, void *data) {
    if(!poller) {
        return -1;
    }
    int ret = poller->add_event(fd, ev_type, data);
    return ret;
}
int frame_path::del_event_from_poller(poller_wrapper *poller, int fd) {
    if(!poller) {
        return -1;
    }
    poller->del_event(fd);
    return 0;
}
int frame_path::mod_event_in_poller(poller_wrapper *poller, int fd, event_type ev_type, void *data) {
    if(!poller) {
        return -1;
    }
    int ret = poller->mod_event(fd, ev_type, data);
    return ret;
}
