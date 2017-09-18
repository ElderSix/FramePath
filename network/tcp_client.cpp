#include "tcp_client.hpp"

#include <arpa/inet.h>

#include <iostream>

using namespace frame_path;

tcp_client::~tcp_client() {
    thread_running = false;
    delete_poller(poller);
    for(auto iter = conns.begin(); iter != conns.end(); iter++) {
        close(iter->first);
        delete(iter->second);
    }
}

int tcp_client::set_connected_cb(std::function<int (socket_client*, int)> func) {
    connected_callback = func;
    return 0;
}

int tcp_client::set_data_in_cb(std::function<int (socket_client*, int)> func) {
    data_in_callback = func;
    return 0;
}

int tcp_client::set_data_out_cb(std::function<int (socket_client*, int)> func) {
    data_out_callback = func;
    return 0;
}

int tcp_client::set_err_cb(std::function<int (socket_client*, void*)> func) {
    error_callback = func;
    return 0;
}

int tcp_client::set_hup_cb(std::function<int (socket_client*, void*)> func) {
    hangup_callback = func;
    return 0;
}

int tcp_client::poller_dispatcher(int ev_type, void* data) {
    int ret = -1;
    client_connection *conn = (client_connection *)data;
    int fd = conn->fd;
    std::cout<<"Receive event "<<ev_type<<" of fd "<<fd<<std::endl;
    switch(ev_type) {
        case EV_READ:
            ret = data_in_callback(this, fd);
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

int tcp_client::run() {
    //FIXME: only run once
    std::function<int(int, void*)> dispatcher = std::bind(&tcp_client::poller_dispatcher, this,
                                                std::placeholders::_1, std::placeholders::_2);
    poller = new_poller(POLLER_EPOLL, dispatcher);
    if(!poller) {
        std::cout<<"Create poller failed"<<std::endl;
        return -1;
    }
    thread_running = true;
    //Create thread for polling
    runner_thread = new std::thread([this](){
        int ret;
        while(this->thread_running) {
            std::cout<<"TCP Client started. Waiting for events..."<<std::endl;
            ret = this->poller->process_events(5000);
            if(ret != 0) {
                break;
            }
        }
        std::cout<<"Client poller thread exits"<<std::endl;
    });
    runner_thread->detach();
    return 0;
}

int tcp_client::connect(client_connection *conn_info) {
    if(!poller) {
        std::cout<<"Poller need to be initialized"<<std::endl;
        return -1;
    }
    //FIXME: check if fd is duplicated

    conn_info->fd = socket(AF_INET, SOCK_STREAM, 0);
    if(conn_info->fd < 0) {
        return -1;
    }
    sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(conn_info->server_port);
    //FIXME: 检查返回值
    server_addr.sin_addr.s_addr = inet_addr(conn_info->server_addr.c_str());

    if((conn_info->client_addr != "")&&(conn_info->client_port != 0)) {
        sockaddr_in client_addr;
        client_addr.sin_family = AF_INET;
        client_addr.sin_port = htons(conn_info->client_port);
        //FIXME: 检查返回值
        client_addr.sin_addr.s_addr = inet_addr(conn_info->client_addr.c_str());
        if(bind(conn_info->fd, (sockaddr *)&client_addr, sizeof(client_addr)) < 0) {
            std::cout<<"Bind local address failed"<<std::endl;
            return -1;
        }
    }

    //使用socket的connect函数
    if(::connect(conn_info->fd, (sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        std::cout<<"Connect to server failed"<<std::endl;
        return -1;
    }
    std::cout<<"Connect to server "<<std::string(inet_ntoa(server_addr.sin_addr))<<" ok, fd is "<<conn_info->fd<<std::endl;
    if(connected_callback(this, conn_info->fd) < 0) {
        close(conn_info->fd);
        return -1;
    }
    add_event_to_poller(poller, conn_info->fd, conn_info->ev_type, conn_info);
    conns[conn_info->fd] = conn_info;
    return 0;
}

int tcp_client::read(int fd, char *dst_buff, int size_to_read) {
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

int tcp_client::write(int fd, const char *src_buff, int size_to_write) {
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
