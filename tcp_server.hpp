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
                data_in_callback([this](void *args){return -1;}),
                data_out_callback([this](void *args){return -1;}),
                error_callback([this](void *args){return -1;}),
                server_ip(htonl(INADDR_ANY)),
                server_port(-1),
                poller(nullptr),
                listen_fd(-1) {};
    virtual ~tcp_server();
    virtual int set_params(std::string param_name, std::string val);
    virtual int set_callback(std::string event_name, std::function<int (void*)> func);
    virtual int run();

private:
    std::function<int (void*)> connected_callback;
    std::function<int (void*)> data_in_callback;
    std::function<int (void*)> data_out_callback;
    std::function<int (void*)> error_callback;
    int poller_dispatcher(int ev_type, void *arg);

    const std::map<std::string, int> server_param_type_map = {
        {"port", SERVER_PARAM_PORT}
    };
    const std::map<std::string, int> server_cb_type_map = {
        {"connected", SERVER_CB_CONNECTED},
        {"data_in", SERVER_CB_DATA_IN},
        {"data_out", SERVER_CB_DATA_OUT}
    };
    int get_param_id(std::string param_name) {
        int ret = -1;
        try {
            ret = server_param_type_map.at(param_name);
        }catch(std::out_of_range e) {

        }
        return ret;
    }
    int get_cb_id(std::string event_name) {
        int ret = -1;
        try {
            ret = server_cb_type_map.at(event_name);
        }catch(std::out_of_range e) {

        }
        return ret;
    }

    unsigned int server_ip;
    unsigned short server_port;
    poller_wrapper *poller;
    int listen_fd;
    //TODO: provide dispatcher
};

}

#endif
