OBJS_DIR = ./objs
LIBS_DIR = ../build/libs
BIN_DIR = ../build/bin
SRCS = $(wildcard *.cpp)
OBJS = $(patsubst %.cpp, $(OBJS_DIR)/%.o, $(notdir $(SRCS)))
LIBS = $(LIBS_DIR)/libframepath_network.a
LIBS += -lpthread

TARGET1 = tcp_echo_server
TARGET2 = tcp_echo_client

CC = g++
AR = ar
CPPFLAGS = -std=c++11
INCLUDE = -I../network/

all: $(BIN_DIR)/$(TARGET1) $(BIN_DIR)/$(TARGET2)

$(BIN_DIR)/$(TARGET1):$(OBJS_DIR)/$(TARGET1).o
	$(CC) $^ $(LIBS) -o $@

$(BIN_DIR)/$(TARGET2):$(OBJS_DIR)/$(TARGET2).o
	$(CC) $^ $(LIBS) -o $@

$(OBJS_DIR)/%.o:%.cpp
	$(CC) $(CPPFLAGS) $(INCLUDE) -c $< -o $@

.PHONY:clean

clean:
	find $(OBJS_DIR) -name *.o -exec rm -rf {} \;
	rm $(BIN_DIR)/$(TARGET1) $(BIN_DIR)/$(TARGET2)
