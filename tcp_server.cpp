#include "tcp_server.hpp"

#include <iostream>

using namespace frame_path;

//Utils:
int str_to_int(std::string s) {

}
//End of utils

tcp_server::~tcp_server() {
    delete_poller(poller);
    close(listen_fd);
}

int tcp_server::set_params(std::string param_name, std::string value) {
    int ret = -1;
    //server_param_type_map是常量成员，只能由常量成员函数使用
    switch (get_param_id(param_name)) {
        case SERVER_PARAM_PORT:
            //FIXME: stoi失败会抛出异常
            server_port = htons(std::stoi(value));
            ret = 0;
            break;
        default:
            //FIXME: param is not supported
            ret = -1;
            break;
    }
    return ret;
}

int tcp_server::set_callback(std::string event_name, std::function<int(void*)> cb) {
    int ret = -1;
    switch (get_cb_id(event_name)) {
        case SERVER_CB_CONNECTED:
            connected_callback = cb;
            ret = 0;
            break;
        case SERVER_CB_DATA_IN:
            data_in_callback = cb;
            ret = 0;
            break;
        case SERVER_CB_DATA_OUT:
            data_out_callback = cb;
            ret = 0;
            break;
        default:
            //FIXME: param is not supported
            ret = -1;
            break;
    }
    return ret;
}

int tcp_server::poller_dispatcher(int ev_type, void* data) {
    int ret = -1;
    connection *conn = (connection *)data;
    int fd = conn->fd;
    switch(ev_type) {
        case EV_READ:
            if(fd == listen_fd) {
                ret = connected_callback(data);
            }else {
                ret = data_in_callback(data);
            }
            break;
        case EV_WRITE:
            ret = data_out_callback(data);
            break;
        case EV_ERR:
            ret = error_callback(data);
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
    //                                            {return poller_dispatcher(ev_type, data);};
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
    new_conn->poller = poller;
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
