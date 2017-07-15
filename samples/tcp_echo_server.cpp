#include "socket_server.hpp"

#include <iostream>

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
    n = server->write(fd, rbuf, 64);
    return 0;
}

int data_out(socket_server* server, int fd) {
    //Data has been write to connection.
    cout<<"[Main app]:Send data ok."<<endl;
    return 0;
}

int main() {
    cout<<"[Main app]:Create tcp server"<<endl;
    socket_server *serv = socket_server::create_server(RAW_TCP_SRV);

    cout<<"[Main app]:Set server port"<<endl;
    serv->set_params("port", "1314");

    cout<<"[Main app]:Set server callback"<<endl;
    serv->set_connected_cb(handle_new_connection);
    serv->set_data_in_cb(data_in);
    //serv->set_data_out_cb(data_out);

    cout<<"[Main app]:Start server"<<endl;
    serv->run();
    return 0;
}
