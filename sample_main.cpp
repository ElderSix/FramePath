#include <iostream>
#include <string>
//for socket
#include <sys/socket.h>
//for sockaddr_in
#include <netinet/in.h>

#include <unistd.h>
#include <fcntl.h>
#include "poller_wrapper.hpp"

using std::cin;
using std::cout;
using std::endl;

#define MAX_READ_BUF_SIZE 4096
#define MAX_WRITE_BUF_SIZE 4096

struct connection {
    int fd;
    bool is_listen_fd;
    poller_wrapper* poller;
    event_type ev_type;
    char rbuf[MAX_READ_BUF_SIZE];
    char wbuf[MAX_WRITE_BUF_SIZE];
};

//-----Poll event handler start-------
int ev_write_handler(void* data);
int ev_read_handler(void* data);

int ev_write_handler(void* data) {
    connection *conn = (connection *)data;
    poller_wrapper *poller = conn->poller;
    cout<<"Unset EPOLLOUT event"<<endl;
    mod_event_in_poller(poller, conn->fd, conn->ev_type, conn);
    return 0;
}

int ev_read_handler(void* data) {
    connection *conn = (connection *)data;
    poller_wrapper *poller = conn->poller;
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
        new_conn->poller = poller;
        new_conn->ev_type = EV_READ_LEVEL;
        add_event_to_poller(poller, new_conn->fd, new_conn->ev_type, new_conn);
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
            del_event_from_poller(poller, conn->fd);
            close(conn->fd);
            delete conn;
            return 0;
        }
        conn->rbuf[n - 1] = '\0';
        std::string str(conn->rbuf);
        cout<<"Receive message"<<"["<<str.size()<<"]: "<<str<<endl;
        if(str.substr(0, 4).compare("quit") == 0) {
            cout<<"Quit connect"<<endl;
            del_event_from_poller(poller, conn->fd);
            close(conn->fd);
            delete conn;
            return 0;
        }
        if(str.substr(0, 3).compare("get") == 0) {
            int l;
            l = send(fd, conn->wbuf, MAX_WRITE_BUF_SIZE, 0);
            cout<<"Send finished."<<endl;
            cout<<"Set EPOLLOUT event"<<endl;
            mod_event_in_poller(poller, conn->fd, conn->ev_type, conn);
            return 0;
        }
        send(fd, conn->rbuf, n , 0);
        return 0;
    }
}
//-----Poll event handler end-------

int ev_write_et_handler(void* data) {
    cout<<"EPOLLOUT ET event"<<endl;
    return 0;
}

int ev_read_et_handler(void* data) {
    connection *conn = (connection *)data;
    poller_wrapper *poller = conn->poller;
    int fd = conn->fd;

    if(conn->is_listen_fd) {
        //ET mode accept
        //todo
        int conn_fd;
        while((conn_fd = accept4(fd, nullptr, nullptr, SOCK_NONBLOCK)) >= 0) {
            cout<<"Connected with nonblock et mode, fd is "<<conn_fd<<endl;
            connection* new_conn = new connection;
            new_conn->fd = conn_fd;
            new_conn->is_listen_fd = false;
            new_conn->poller = poller;
            new_conn->ev_type = EV_RW;
            add_event_to_poller(poller, new_conn->fd, new_conn->ev_type, new_conn);
        }
        if((conn_fd == -1)&&(errno != EAGAIN)) {
            cout<<"Accept fail: "<<errno<<endl;
            return -1;
        }
        return 0;
    }else {
        //ET mode:
        int n = 0;
        int to_read = MAX_READ_BUF_SIZE;
        //ET mode read, read until error or EAGAIN
        while(((n = recv(fd, conn->rbuf + n, to_read, 0)) > 0) && (to_read > 0)) {
            to_read -= n;
        }
        if((n == -1)&&(errno != EAGAIN)) {
            cout<<"recv error"<<endl;
            return -1;
        }

        n = MAX_READ_BUF_SIZE - to_read;
        conn->rbuf[n] = '\0';
        std::string str(conn->rbuf);
        cout<<"Receive message"<<"["<<str.size()<<"]: "<<str<<endl;
        if(str.substr(0, 4).compare("quit") == 0) {
            cout<<"Quit connect"<<endl;
            del_event_from_poller(poller, conn->fd);
            close(conn->fd);
            delete conn;
            return 0;
        }

        //Echo back to client
        for(int i = 0; i < n; i++) {
            conn->wbuf[i] = conn->rbuf[i];
        }
        //ET mode write, write until error or EAGAIN, or all the data have been sent
        int n_sent = 0;
        while(((n_sent = send(fd, conn->wbuf + n_sent, n, 0)) > 0) && (n > 0)) {
            n -= n_sent;
        }
        if((n_sent == -1)&&(errno != EAGAIN)) {
            cout<<"Send error"<<endl;
        }
        return 0;
    }
}

int ev_err_handler(void* data) {
    cout<<"Error event occurred"<<endl;
    return 0;
}

int event_dispatcher(int ev_type, void *data) {
    switch(ev_type) {
        case EV_READ:
            ev_read_et_handler(data);
            break;
        case EV_WRITE:
            ev_write_et_handler(data);
            break;
        case EV_ERR:
            ev_err_handler(data);
            break;
        default:
            break;
    }
    return 0;
}

int main() {
    //new epoll_wrapper
    poller_wrapper *poller = new_poller(POLLER_EPOLL);
    if(!poller) {
        return -1;
    }

    //create socket
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    cout<<"Create listen fd: "<<fd<<endl;

    //set socket nonblocking
    int opts = fcntl(fd, F_GETFL);
    if(opts < 0) {
        return -1;
    }
    opts |= O_NONBLOCK;
    if(fcntl(fd, F_SETFL, opts) < 0) {
        return -1;
    }

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
    if(-1 == poller->create_poller(10, event_dispatcher)) {
        cout<<"Create epoll fail"<<endl;
        return 0;
    }
    //add fd and handler to epoll_wrapper
    connection* new_conn = new connection;
    new_conn->fd = fd;
    new_conn->is_listen_fd = true;
    new_conn->poller = poller;
    new_conn->ev_type = EV_READ;
    add_event_to_poller(poller, new_conn->fd, new_conn->ev_type, new_conn);
    //loop for process_event
    int ret;
    while(1) {
        ret = poller->process_events(5000);
        cout<<"Waiting events..."<<endl;
        if(ret != 0) {
            break;
        }
    }
    cout<<"Quit process"<<endl;

    //todo: release fd
    delete_poller(poller);
    return 0;
}
