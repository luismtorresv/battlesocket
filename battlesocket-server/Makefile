CC = gcc
CFLAGS = -Wall -Wextra -pthread
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

memcheck: $(OBJ)
	$(CC) $(ASANFLAGS) $(CFLAGS) -o $@ $^
	./$@
	@echo "Memory check passed"

clean:
	rm -f $(OBJ) $(TARGET)

.PHONY: all clean
