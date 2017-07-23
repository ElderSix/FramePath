#include "socket_client.hpp"
#include "tcp_client.hpp"

using namespace frame_path;

socket_client* socket_client::create_client(client_type type) {
    socket_client *client = nullptr;
    switch(type) {
        case RAW_TCP_CLT:
            try {
                client = new tcp_client;
            }catch(const std::bad_alloc& e) {
                //TODO
            }
        default:
            break;
    }
    return client;
}
