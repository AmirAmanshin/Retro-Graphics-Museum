CXX = g++
CC = gcc

CXXFLAGS = -std=c++17 -Iinclude -Wall -Wextra -g
CFLAGS = -Iinclude -g

LDFLAGS = -lglfw -lGL -ldl

TARGET = app

CXX_SRCS = $(wildcard src/*.cpp)
C_SRCS = $(wildcard src/*.c)

OBJS = $(CXX_SRCS:.cpp=.o) $(C_SRCS:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(OBJS) -o $(TARGET) $(LDFLAGS)

src/%.o: src/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

src/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f src/*.o $(TARGET)