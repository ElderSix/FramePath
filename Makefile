all:
	g++ poller_wrapper.cpp epoll_wrapper.cpp sample_main.cpp -std=c++11 -o sample_main
