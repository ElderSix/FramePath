#include "tcp_server.hpp"

#include <iostream>

using namespace frame_path;


tcp_server::~tcp_server() {
    delete_poller(poller);
    close(listen_fd);
}

int tcp_server::set_params(const std::string &param_name, const std::string &value) {
    int ret = -1;
    //server_param_type_map是常量成员，只能由常量成员函数使用
    switch (get_param_id(param_name)) {
        case SERVER_PARAM_PORT:
            //FIXME: stoi失败会抛出异常
            server_port = std::stoi(value);
            ret = 0;
            break;
        default:
            //FIXME: param is not supported
            ret = -1;
            break;
    }
    return ret;
}

int tcp_server::set_connected_cb(std::function<int (void*)> func) {
    connected_callback = func;
    return 0;
}
int tcp_server::set_data_in_cb(std::function<int (socket_server*, int)> func) {
    data_in_callback = func;
    return 0;
}
int tcp_server::set_data_out_cb(std::function<int (socket_server*, int)> func) {
    data_out_callback = func;
    return 0;
}
int tcp_server::set_err_cb(std::function<int (socket_server*, void*)> func) {
    error_callback = func;
    return 0;
}
int tcp_server::set_hup_cb(std::function<int (socket_server*, void*)> func) {
    hangup_callback = func;
    return 0;
}

int tcp_server::poller_dispatcher(int ev_type, void* data) {
    int ret = -1;
    connection *conn = (connection *)data;
    int fd = conn->fd;
    std::cout<<"Receive event "<<ev_type<<std::endl;
    switch(ev_type) {
        case EV_READ:
            if(fd == listen_fd) {
                //Connection OK, save it to conns
                int conn_fd;
                while((conn_fd = accept4(fd, nullptr, nullptr, SOCK_NONBLOCK)) >= 0) {
                    std::cout<<"Connected with nonblock et mode, fd is "<<conn_fd<<std::endl;
                    ret = connected_callback(data);
                    if(ret >= 0) {
                        connection* new_conn = new connection;
                        new_conn->fd = conn_fd;
                        new_conn->is_listen_fd = false;
                        new_conn->ev_type = EV_RW;
                        add_event_to_poller(poller, new_conn->fd, new_conn->ev_type, new_conn);
                        conns[conn_fd] = new_conn;
                    }else {
                        close(conn_fd);
                    }
                }
                if((conn_fd == -1)&&(errno != EAGAIN)) {
                    std::cout<<"Accept fail: "<<errno<<std::endl;
                    return -1;
                }
            }else {
                ret = data_in_callback(this, fd);
            }
            break;
        case EV_WRITE:
            //TODO: Close fd
            ret = data_out_callback(this, fd);
            break;
        case EV_ERR:
            ret = error_callback(this, data);
            del_event_from_poller(poller, fd);
            close(fd);
            conns.erase(fd);
            break;
        case EV_HUP:
            std::cout<<"Remote hang up the connection."<<std::endl;
            del_event_from_poller(poller, fd);
            close(fd);
            conns.erase(fd);
            break;
        default:
            break;
    }
    return ret;
}

int tcp_server::run() {
    listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    std::cout<<"Create listen fd: "<<listen_fd<<std::endl;
    if(listen_fd < 0) {
        return -1;
    }

    int sock_opts = fcntl(listen_fd, F_GETFL);
    if(sock_opts < 0) {
        return -1;
    }
    sock_opts |= O_NONBLOCK;
    if(0 > fcntl(listen_fd, F_SETFL, sock_opts)) {
        return -1;
    }

    sockaddr_in saddr;
    saddr.sin_family = AF_INET;
    saddr.sin_addr.s_addr = htonl(INADDR_ANY);
    saddr.sin_port = htons(server_port);

    if(-1 == bind(listen_fd, (sockaddr *)&saddr, sizeof(saddr))) {
        close(listen_fd);
        return -1;
    }

    //FIXME: fix magic number 10
    if(-1 == listen(listen_fd, 10)) {
        close(listen_fd);
        return -1;
    }

    //std::function<int(int, void*)> dispatcher = [this](int ev_type, void *data)
    //                                            {return this->poller_dispatcher(ev_type, data);};
    std::function<int(int, void*)> dispatcher = std::bind(&tcp_server::poller_dispatcher, this,
                                                std::placeholders::_1, std::placeholders::_2);
    poller = new_poller(POLLER_EPOLL, dispatcher);
    if(!poller) {
        close(listen_fd);
        return -1;
    }

    connection* new_conn = new connection;
    new_conn->fd = listen_fd;
    new_conn->is_listen_fd = true;
    new_conn->ev_type = EV_READ;
    add_event_to_poller(poller, new_conn->fd, new_conn->ev_type, new_conn);

    int ret;
    while(1) {
        ret = poller->process_events(5000);
        std::cout<<"TCP Server started. Waiting for events..."<<std::endl;
        if(ret != 0) {
            break;
        }
    }

    //todo: release fd
    delete_poller(poller);
    close(listen_fd);

    return 0;
}

int tcp_server::read(int fd, char *dst_buff, int size_to_read) {
    int n_per_time = 0;
    int n_read = 0;
    //ET mode read, read until error or EAGAIN
    while((size_to_read > 0)&&((n_per_time = recv(fd, dst_buff + n_read, size_to_read, 0)) > 0)) {
        size_to_read -= n_per_time;
        n_read += n_per_time;
    }
    if((n_per_time == -1)&&(errno != EAGAIN)) {
        std::cout<<"Recv error"<<std::endl;
        return -1;
    }
    return n_read;
}

int tcp_server::write(int fd, char *src_buff, int size_to_write) {
    int n_per_time = 0;
    int n_sent = 0;
    while((size_to_write > 0)&&((n_per_time = send(fd, src_buff + n_sent, size_to_write, 0)) > 0)) {
        size_to_write -= n_per_time;
        n_sent += n_per_time;
    }
    if((n_per_time == -1)&&(errno != EAGAIN)) {
        std::cout<<"Send error"<<std::endl;
    }
    return n_sent;
}
