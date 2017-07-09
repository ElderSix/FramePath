all:
	g++ poller_wrapper.cpp epoll_wrapper.cpp socket_server.cpp tcp_server.cpp main.cpp -std=c++11 -o sample_main
