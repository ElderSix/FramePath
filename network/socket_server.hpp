/*
poller-->socket_server-->protocol-->request-->dispatcher
poller的事件处理函数由socket_server模块提供
socket_server的数据处理函数由protocol模块提供
*/

#ifndef _SOCKET_SERVER_HPP_
#define _SOCKET_SERVER_HPP_

#include "poller_wrapper.hpp"

#include <functional>
#include <string>

namespace frame_path {

#define MAX_READ_BUF_SIZE 4096
#define MAX_WRITE_BUF_SIZE 4096

enum server_type {
    RAW_TCP_SRV,
    RAW_UDP_SRV,
    HTTP_SRV
};

enum server_status {
    SRV_RUNNING,
    SRV_STOPPED,
    SRV_UNKNOWN
};

struct connection {
    int fd;
    bool is_listen_fd;
    frame_path::event_type ev_type;
};

//父类
//子类是tcp、udp、http、rtsp等等, 对应的protocol_parser不同，其数据处理也不同
class socket_server {
public:
    virtual ~socket_server() {}
    static socket_server* create_server(server_type type);
    virtual int set_params(const std::string &name, const std::string &val) = 0;
    virtual int set_connected_cb(std::function<int (void*)> func) = 0;
    virtual int set_data_in_cb(std::function<int (socket_server*, int)> func) = 0;
    virtual int set_data_out_cb(std::function<int (socket_server*, int)> func) = 0;
    virtual int set_err_cb(std::function<int (socket_server*, void*)> func) = 0;
    virtual int set_hup_cb(std::function<int (socket_server*, void*)> func) = 0;
    virtual int run() = 0;
    virtual int read(int fd, char *dst_buff, int len_to_read) = 0;
    virtual int write(int fd, char *src_buff, int len_to_write) = 0;
    //protocol_parser pm; //负责处理事件的protocol模块，需要在构造函数初始化
};

}
#endif
