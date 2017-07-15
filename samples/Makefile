OBJS_DIR = ./objs
LIBS_DIR = ../build/libs
BIN_DIR = ../build/bin
SRCS = $(wildcard *.cpp)
OBJS = $(patsubst %.cpp, $(OBJS_DIR)/%.o, $(notdir $(SRCS)))
LIBS = $(LIBS_DIR)/libframepath_network.a

TARGET = tcp_server

BIN_TARGET = $(BIN_DIR)/$(TARGET)

CC = g++
AR = ar
CPPFLAGS = -std=c++11
INCLUDE = -I../network/

$(BIN_TARGET):$(OBJS)
	$(CC) $(OBJS) $(LIBS) -o $@

$(OBJS_DIR)/%.o:%.cpp
	$(CC) $(CPPFLAGS) $(INCLUDE) -c $< -o $@

.PHONY:clean

clean:
	find $(OBJS_DIR) -name *.o -exec rm -rf {} \;
	rm $(BIN_TARGET)
