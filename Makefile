CC = gcc
CFLAGS = -Wall -Iinclude
SRC = src/main.c src/decoder.c src/registers.c
OBJ = $(SRC:.c=.o)
TARGET = strace_tracer

all: $(TARGET)

$(TARGET): $(OBJ)
	gcc src/main.c src/decoder.c src/registers.c -Iinclude -o strace_tracer

clean:
	rm -f src/*.o $(TARGET)
