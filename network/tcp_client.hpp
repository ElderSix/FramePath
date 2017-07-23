#ifndef _TCP_CLIENT_HPP_
#define _TCP_CLIENT_HPP_

#include "socket_client.hpp"
//for socket
#include <sys/socket.h>
//for sockaddr_in
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>

#include <map>
#include <thread>

namespace frame_path {

class tcp_client : public socket_client {
public:
    tcp_client():connected_callback([this](socket_client* s, int fd){return -1;}),
                data_in_callback([this](socket_client* s, int fd){return -1;}),
                data_out_callback([this](socket_client* s, int fd){return -1;}),
                error_callback([this](socket_client* s, void *args){return -1;}),
                hangup_callback([this](socket_client* s, void *args){return -1;}),
                poller(nullptr),
                runner_thread(nullptr),
                thread_running(false) {};
    virtual ~tcp_client();
    //virtual int set_params(const std::string &name, const std::string &val) = 0;
    virtual int set_connected_cb(std::function<int (socket_client*, int)> func);
    virtual int set_data_in_cb(std::function<int (socket_client*, int)> func);
    virtual int set_data_out_cb(std::function<int (socket_client*, int)> func);
    virtual int set_err_cb(std::function<int (socket_client*, void*)> func);
    virtual int set_hup_cb(std::function<int (socket_client*, void*)> func);
    virtual int run();  //Run poller
    virtual int connect(client_connection *conn_info);
    virtual int read(int fd, char *dst_buff, int len_to_read);
    virtual int write(int fd, const char *src_buff, int len_to_write);
private:
    std::function<int (socket_client* s, int)> connected_callback;
    std::function<int (socket_client* s, int)> data_in_callback;
    std::function<int (socket_client* s, int)> data_out_callback;
    std::function<int (socket_client* s, void*)> error_callback;
    std::function<int (socket_client* s, void*)> hangup_callback;
    int poller_dispatcher(int ev_type, void *arg);

    poller_wrapper *poller;
    std::thread *runner_thread;
    bool thread_running;
    std::map<int, client_connection*> conns;
};

}

#endif
