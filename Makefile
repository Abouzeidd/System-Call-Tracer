CC = gcc

CFLAGS = -Wall -Iinclude

SRC = \
	src/main.c \
	src/decoder.c \
	src/registers.c \
	src/syscalls.c \
	src/formatter.c \
	src/output.c

OBJ = $(SRC:.c=.o)

TARGET = strace_tracer

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(OBJ) $(CFLAGS) -o $(TARGET)

clean:
	rm -f src/*.o $(TARGET)
