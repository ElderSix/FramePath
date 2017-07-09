#include "socket_server.hpp"
#include "tcp_server.hpp"

using namespace frame_path;

socket_server* socket_server::create_server(server_type type) {
    socket_server *server = nullptr;
    switch(type) {
        case RAW_TCP_SRV:
            try {
                server = new tcp_server;
            }catch(const std::bad_alloc& e) {
                //TODO
            }
        default:
            break;
    }
    return nullptr;
}
