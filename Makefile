CXX = g++
CC = gcc

CXXFLAGS = -std=c++17 -Iinclude -Wall -Wextra -g -MMD -MP
CFLAGS = -Iinclude -g -MMD -MP

rwildcard = $(foreach d,$(wildcard $(1:=/*)),$(call rwildcard,$d,$2) $(filter $(subst *,%,$2),$d))

CXX_SRCS = $(call rwildcard,src,*.cpp)
C_SRCS = $(call rwildcard,src,*.c)

OBJS = $(CXX_SRCS:.cpp=.o) $(C_SRCS:.c=.o)
DEPS = $(OBJS:.o=.d)

ifeq ($(OS),Windows_NT)
    TARGET = app.exe
    LDFLAGS = -lglfw3 -lgdi32 -lopengl32
    RM_CMD = del /Q /F
    CLEAN_OBJS = $(subst /,\,$(OBJS))
    CLEAN_DEPS = $(subst /,\,$(DEPS))
else
    TARGET = app
    LDFLAGS = -lglfw -lGL -ldl
    RM_CMD = rm -f
    CLEAN_OBJS = $(OBJS)
    CLEAN_DEPS = $(DEPS)
endif

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(OBJS) -o $(TARGET) $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	-$(RM_CMD) $(CLEAN_OBJS)
	-$(RM_CMD) $(CLEAN_DEPS)
	-$(RM_CMD) $(TARGET)

-include $(DEPS)