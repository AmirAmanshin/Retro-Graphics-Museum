CXX = g++
CC = gcc

CXXFLAGS = -std=c++17 -Iinclude -Wall -Wextra -g
CFLAGS = -Iinclude -g

# 1. Кроссплатформенная рекурсивная функция поиска файлов вместо shell find
rwildcard = $(foreach d,$(wildcard $(1:=/*)),$(call rwildcard,$d,$2) $(filter $(subst *,%,$2),$d))

CXX_SRCS = $(call rwildcard,src,*.cpp)
C_SRCS = $(call rwildcard,src,*.c)

OBJS = $(CXX_SRCS:.cpp=.o) $(C_SRCS:.c=.o)

# 2. Определение ОС и настройка платформозависимых переменных
ifeq ($(OS),Windows_NT)
    TARGET = app.exe
    LDFLAGS = -lglfw3 -lgdi32 -lopengl32
    # Настройки для очистки в командной строке Windows (cmd)
    RM_CMD = del /Q /F
    CLEAN_OBJS = $(subst /,\,$(OBJS))
else
    TARGET = app
    LDFLAGS = -lglfw -lGL -ldl
    # Настройки для очистки в Linux
    RM_CMD = rm -f
    CLEAN_OBJS = $(OBJS)
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
	-$(RM_CMD) $(TARGET)