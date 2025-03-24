CC = gcc
CFLAGS = -Wall -Wextra -pthread
INCLUDES = -I./include
SRC = $(wildcard src/*.c)
OBJ = $(SRC:.c=.o)
TARGET = server_exec

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)

.PHONY: all clean
