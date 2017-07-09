#include "socket_server.hpp"

#include <iostream>

using std::cin;
using std::cout;
using std::endl;

using namespace frame_path;

int handle_new_connection(void *arg) {
    //Check if the connection is valid, if not, return False
    cout<<"Get connection"<<endl;
    return 0;
}

int read_data(void *arg) {
    //Read data from connection
    cout<<"Receive data"<<endl;
    return 0;
}

int main() {
    socket_server *serv = socket_server::create_server(RAW_TCP_SRV);
    serv->set_params("port", "1314");
    serv->set_callback("connected", handle_new_connection);
    serv->set_callback("data_in", read_data);
    serv->run();
    return 0;
}
