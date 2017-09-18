#include "socket_server.hpp"

#include <iostream>
#include <thread>
#include <vector>

using std::cin;
using std::cout;
using std::endl;

using namespace frame_path;

int handle_new_connection(void *arg) {
    //Check if the connection is valid, if not, return False
    cout<<"[Main app]:Get connection"<<endl;
    return 0;
}

int data_in(socket_server* server, int fd) {
    //Read and echo data to client
    char rbuf[64];
    int n = server->read(fd, rbuf, 64);
    cout<<"[Main app]:Receive data of "<<n<<" bytes"<<endl;
    n = server->write(fd, rbuf, n);
    return 0;
}

int data_out(socket_server* server, int fd) {
    //Data has been write to connection.
    cout<<"[Main app]:Send data ok."<<endl;
    return 0;
}

int main() {
    cout<<"[Main app]:Create tcp server"<<endl;
    socket_server *serv;
    std::vector<socket_server *> servers;
    int server_num = 3;
    for(int i = 0; i < server_num; i++) {
        serv = socket_server::create_server(RAW_TCP_SRV);
        serv->set_params("port", "1314");
        serv->set_connected_cb(handle_new_connection);
        serv->set_data_in_cb(data_in);
        servers.push_back(serv);
    }

    cout<<"[Main app]:Start server with "<<server_num<<" thread."<<endl;
    std::vector<std::thread> threads;
    for(int i = 0; i < server_num; i++) {
        threads.push_back(std::thread(std::bind(&socket_server::run, servers[i])));
    }
    for(int i = 0; i < server_num; i++) {
        threads[i].join();
    }
    return 0;
}
