#include <iostream>
#include <string>
//for socket
#include <sys/socket.h>
//for sockaddr_in
#include <netinet/in.h>
#include "epoll_wrapper.h"

using std::cin;
using std::cout;
using std::endl;

#define MAX_READ_BUF_SIZE 1024
#define MAX_WRITE_BUF_SIZE 1024

struct connection {
    int fd;
    bool is_listen_fd;
    epoll_wrapper* epoller;
    event_entry* evt;
    char rbuf[MAX_READ_BUF_SIZE];
    char wbuf[MAX_WRITE_BUF_SIZE];
};

int ev_write_handler(void* data);
int ev_read_handler(void* data);

int ev_write_handler(void* data) {
    connection *conn = (connection *)data;
    epoll_wrapper *epoller = conn->epoller;
    int fd = conn->fd;
    cout<<"Unset EPOLLOUT event"<<endl;
    conn->evt->rev_handler = ev_read_handler;
    epoller->mod_poller(fd, conn->evt, EV_READ);
    return 0;
}

int ev_read_handler(void* data) {
    connection *conn = (connection *)data;
    epoll_wrapper *epoller = conn->epoller;
    int fd = conn->fd;

    if(conn->is_listen_fd) {
        int conn_fd = accept(fd, nullptr, nullptr);
        if(conn_fd == -1) {
            cout<<"Accept fail: "<<errno<<endl;
            return -1;
        }
        cout<<"Connected"<<endl;
        connection* new_conn = new connection;
        new_conn->fd = conn_fd;
        new_conn->is_listen_fd = false;
        new_conn->epoller = epoller;
        new_conn->evt = new event_entry;
        new_conn->evt->rev_handler = ev_read_handler;
        new_conn->evt->user_data = new_conn;
        epoller->add_poller(conn_fd, new_conn->evt, EV_READ);
        return 0;
    }else {
        int n = recv(fd, conn->rbuf, 1024, 0);
        //n == 0, a stream socket peer has performed an orderly shutdown
        //n < 0, an error occurred, errno  is  set  to  indicate the error(maybe EAGAIN).
        if((n < 0) && (errno == EAGAIN)) {
            cout<<"Read again"<<endl;
            return 0;
        }
        if(n == 0) {
            cout<<"Client diconnected"<<endl;
            epoller->del_poller(fd);
            delete conn;
            return 0;
        }
        conn->rbuf[n - 1] = '\0';
        std::string str(conn->rbuf);
        cout<<"Receive message"<<"["<<str.size()<<"]: "<<str<<endl;
        if(str.substr(0, 4).compare("quit") == 0) {
            cout<<"Quit connect"<<endl;
            epoller->del_poller(fd);
            delete conn;
            return 0;
        }
        if(str.substr(0, 3).compare("get") == 0) {
            int l;
            l = send(fd, conn->wbuf, MAX_WRITE_BUF_SIZE, 0);
            cout<<"Send finished."<<endl;
            cout<<"Set EPOLLOUT event"<<endl;
            conn->evt->wev_handler = ev_write_handler;
            epoller->mod_poller(fd, conn->evt, EV_WRITE);
            return 0;
        }
        send(fd, conn->rbuf, n , 0);
        return 0;
    }
}

int ev_err_handler(void* data) { 
    cout<<"Error event occurred"<<endl;
    return 0;
}

int main() {
    //new epoll_wrapper
    epoll_wrapper epoller;

    //create socket
    int fd = socket(AF_INET, SOCK_STREAM, 0);

    //bind and listen
    sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(1314);
    if(-1 == bind(fd, (sockaddr*)&server_addr, sizeof(server_addr))) {
        cout<<"Bind error"<<endl;
        return 0;
    }

    //The backlog argument (10) defines the maximum length to which the queue of pending connections for sockfd may grow. 
    if(-1 == listen(fd, 10)) {
        cout<<"Listen error"<<endl;
        return 0;
    }
    if(-1 == epoller.create(10)) {
        cout<<"Create epoll fail"<<endl;
        return 0;
    }
    //add fd and handler to epoll_wrapper
    connection* new_conn = new connection;
    new_conn->fd = fd;
    new_conn->is_listen_fd = true;
    new_conn->epoller = &epoller;
    new_conn->evt = new event_entry;
    new_conn->evt->rev_handler = ev_read_handler;
    new_conn->evt->user_data = new_conn;
    epoller.add_poller(fd, new_conn->evt, EV_READ);
    //loop for process_event
    int ret;
    while(1) {
        ret = epoller.process_events(2000);
        cout<<"Waiting events..."<<endl;
        if(ret > 0) {
            break;
        }
    }
    cout<<"Quit process"<<endl;

    //todo: release fd
    return 0;
}