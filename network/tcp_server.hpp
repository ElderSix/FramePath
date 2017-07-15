#ifndef _TCP_SERVER_HPP_
#define _TCP_SERVER_HPP_

#include "socket_server.hpp"
//for socket
#include <sys/socket.h>
//for sockaddr_in
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>

#include <string>
#include <map>
#include <vector>

namespace frame_path {

enum server_param_type_id {
    SERVER_PARAM_PORT,
    SERVER_PARAM_INVALID
};

enum server_cb_type_id {
    SERVER_CB_CONNECTED,
    SERVER_CB_DATA_IN,
    SERVER_CB_DATA_OUT,
};

class tcp_server : public socket_server {
public:
    tcp_server():connected_callback([this](void *args){return -1;}),
                data_in_callback([this](socket_server* s, int fd){return -1;}),
                data_out_callback([this](socket_server* s, int fd){return -1;}),
                error_callback([this](socket_server* s, void *args){return -1;}),
                hangup_callback([this](socket_server* s, void *args){return -1;}),
                server_ip(htonl(INADDR_ANY)),
                server_port(-1),
                poller(nullptr),
                listen_fd(-1) {};
    virtual ~tcp_server();
    virtual int set_params(const std::string &name, const std::string &val);
    virtual int set_connected_cb(std::function<int (void*)> func);
    virtual int set_data_in_cb(std::function<int (socket_server*, int)> func);
    virtual int set_data_out_cb(std::function<int (socket_server*, int)> func);
    virtual int set_err_cb(std::function<int (socket_server*, void*)> func);
    virtual int set_hup_cb(std::function<int (socket_server*, void*)> func);
    virtual int run();
    virtual int read(int fd, char *dst_buff, int size_to_read);
    virtual int write(int fd, char *src_buff, int size_to_write);

private:
    std::function<int (void*)> connected_callback;
    std::function<int (socket_server* s, int)> data_in_callback;
    std::function<int (socket_server* s, int)> data_out_callback;
    std::function<int (socket_server* s, void*)> error_callback;
    std::function<int (socket_server* s, void*)> hangup_callback;
    int poller_dispatcher(int ev_type, void *arg);

    const std::map<std::string, int> server_param_type_map = {
        {"port", SERVER_PARAM_PORT}
    };
    int get_param_id(std::string param_name) {
        int ret = -1;
        try {
            ret = server_param_type_map.at(param_name);
        }catch(std::out_of_range e) {

        }
        return ret;
    }

    unsigned int server_ip;
    unsigned short server_port;
    poller_wrapper *poller;
    int listen_fd;
    std::map<int, connection *> conns;
};

}

#endif
