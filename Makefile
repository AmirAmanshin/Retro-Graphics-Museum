CXX = g++
CC = gcc

CXXFLAGS = -std=c++17 -Iinclude -Wall -Wextra -g
CFLAGS = -Iinclude -g

LDFLAGS = -lglfw -lGL -ldl

TARGET = app

CXX_SRCS = $(shell find src -name '*.cpp')
C_SRCS = $(shell find src -name '*.c')

OBJS = $(CXX_SRCS:.cpp=.o) $(C_SRCS:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(OBJS) -o $(TARGET) $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)