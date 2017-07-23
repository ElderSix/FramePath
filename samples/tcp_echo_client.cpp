#include "socket_client.hpp"

#include <thread>         // std::this_thread::sleep_for
#include <chrono>

#include <iostream>
#include <vector>
#include <string>
#include <map>

using std::cin;
using std::cout;
using std::endl;

using namespace frame_path;

static void send_message(socket_client* client, int fd, std::string msg) {
    int n = client->write(fd, msg.c_str(), msg.size() + 1);
    cout<<"[Main app]: Send message of "<<n<<" bytes"<<endl;
}

int handle_new_connection(socket_client* client, int fd) {
    cout<<"[Main app]:Get connection"<<endl;
    //send_message(client, fd, "Hello");
    return 0;
}

int data_out(socket_client* client, int fd) {
    cout<<"[Main app]:Send data ok."<<endl;
    return 0;
}

int data_in(socket_client* client, int fd) {
    //Read and echo data to client
    char rbuf[64];
    int n = client->read(fd, rbuf, 64);
    cout<<"[Main app]:Receive data of "<<n<<" bytes"<<endl;
    return 0;
}

int main() {
    cout<<"[Main app]:Create tcp client"<<endl;
    socket_client *client = socket_client::create_client(RAW_TCP_CLT);

    cout<<"[Main app]:Set client callback"<<endl;
    client->set_connected_cb(handle_new_connection);
    client->set_data_in_cb(data_in);
    client->set_data_out_cb(data_out);

    cout<<"[Main app]:Start server run"<<endl;
    client->run();

    cout<<"[Main app]:Connect to server"<<endl;
    std::string server_ip = "192.168.3.11";
    client_connection conn_info = {
        .fd = 0,
        .client_addr = "",
        .server_addr = "192.168.3.11",
        .client_port = 0,
        .server_port = 1314,
        .ev_type = EV_RW
    };
    if(client->connect(&conn_info) < 0) {
        cout<<"[Main app]:Connect to server failed"<<endl;
    }
    while(1) {
        std::this_thread::sleep_for(std::chrono::seconds(5));
        send_message(client, conn_info.fd, "Hello server");
    }
}
