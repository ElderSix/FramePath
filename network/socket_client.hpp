#ifndef _SOCKET_CLIENT_HPP_
#define _SOCKET_CLIENT_HPP_

#include "poller_wrapper.hpp"
#include <functional>
#include <string>

namespace frame_path {

enum client_type {
    RAW_TCP_CLT,
    RAW_UDP_CLT,
    HTTP_CLT
};

enum client_status {
    CLT_RUNNING,
    CLT_STOPPED,
    CLT_UNKNOWN
};


struct client_connection {
    int fd;
    std::string client_addr;
    std::string server_addr;
    unsigned short client_port;
    unsigned short server_port;
    frame_path::event_type ev_type;
};

class socket_client {
public:
    virtual ~socket_client() {}
    static socket_client* create_client(client_type type);
    //virtual int set_params(const std::string &name, const std::string &val) = 0;
    virtual int set_connected_cb(std::function<int (socket_client*, int)> func) = 0;
    virtual int set_data_in_cb(std::function<int (socket_client*, int)> func) = 0;
    virtual int set_data_out_cb(std::function<int (socket_client*, int)> func) = 0;
    virtual int set_err_cb(std::function<int (socket_client*, void*)> func) = 0;
    virtual int set_hup_cb(std::function<int (socket_client*, void*)> func) = 0;
    virtual int run() = 0;
    virtual int connect(client_connection *conn_info) = 0;
    virtual int read(int fd, char *dst_buff, int len_to_read) = 0;
    virtual int write(int fd, const char *src_buff, int len_to_write) = 0;
};

}
#endif
