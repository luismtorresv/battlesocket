CC = gcc

CFLAGS  = -Wall
CFLAGS += -Wextra
CFLAGS += -Werror
CFLAGS += -pthread
CFLAGS += -g

INCLUDES = -I./include
SRC = $(wildcard src/*.c)
OBJ = $(SRC:.c=.o)
TARGET = server

ASANFLAGS  = -fsanitize=address
ASANFLAGS += -fno-common
ASANFLAGS += -fno-omit-frame-pointer

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

memcheck.out: $(OBJ)
	$(CC) $(ASANFLAGS) $(CFLAGS) -o $@ $^
	./$@
	@echo "Memory check passed"

clean:
	rm -f $(OBJ) $(TARGET) *.out

.PHONY: all clean
