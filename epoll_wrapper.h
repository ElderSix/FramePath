#include <sys/epoll.h>
#include <map>

enum event_type{
    EV_READ,
    EV_WRITE,
    EV_READ_AND_WRITE
};

struct event_handler {
    int (*handler)(int, void *);
    void* user_data;
};

class epoll_wrapper {
public:
    epoll_wrapper():epfd(-1),nfds(0) {};
    ~epoll_wrapper() {};
    int create(int n) {
        if(n <= 0) {
            return -1;
        }
        if(epfd > 0) {
            return nfds;
        }
        epfd = epoll_create(n);
        if(epfd <= 0) {
            return -1;
        }
        nfds = n;
        return nfds;
    }
    int add_poller(int fd, struct event_handler* ev_handler, int ev_type);
    int del_poller(int);
    int mod_poller(int fd, struct event_handler* ev_handler, int ev_type);
    //caller release events
    int process_events(int time_wait);
private:
    struct epoll_event* make_ev(int fd, int ev_type) {
        struct epoll_event* ev = new struct epoll_event;
        ev->data.fd = fd;
        switch(ev_type) {
            case EV_READ:
                ev->events = EPOLLIN;
                break;
            case EV_WRITE:
                ev->events = EPOLLOUT;
                break;
            case EV_READ_AND_WRITE:
                ev->events = EPOLLIN|EPOLLOUT;
                break;
            default:
                delete ev;
                //注意，NULL不是关键字，nullptr则在C++11成为关键字
                //iostream里定义了NULL，所以不包含iostream的话用NULL会编译失败
                ev = nullptr;
        }
        return ev;
    }
    void err_ev_process(int fd);
    void read_ev_process(int fd);
    void write_ev_handler(int fd);
    int epfd;
    int nfds;
    std::map<int, struct event_handler*> read_ev_handlers;
    std::map<int, struct event_handler*> write_ev_handlers;
};