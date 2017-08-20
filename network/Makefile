OBJS_DIR = ./objs
LIBS_DIR = ../build/libs
SRCS = $(wildcard *.cpp)
OBJS = $(patsubst %.cpp, $(OBJS_DIR)/%.o, $(notdir $(SRCS)))

TARGET = libframepath_network.a

LIB_TARGET = $(LIBS_DIR)/$(TARGET)

CC = g++
AR = ar
CPPFLAGS = -std=c++11

all: dir_target $(LIB_TARGET)

dir_target:
	mkdir -p $(OBJS_DIR)

$(LIB_TARGET):$(OBJS)
	$(AR) -rc $@ $^

$(OBJS_DIR)/%.o:%.cpp
	$(CC) $(CPPFLAGS) -c $< -o $@

.PHONY:clean

clean:
	find $(OBJS_DIR) -name *.o -exec rm -rf {} \;
	rm $(LIB_TARGET)
