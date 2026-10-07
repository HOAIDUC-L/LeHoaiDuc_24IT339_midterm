CC = cc
CFLAGS = -Wall -Wextra -Werror -Iinclude -D_POSIX_C_SOURCE=200809L -D_DEFAULT_SOURCE -D_NETBSD_SOURCE
LDFLAGS =

SRCS = src/main.c \
       src/options.c \
       src/file_info.c \
       src/display.c \
       src/sort.c \
       src/traverse.c

OBJS = $(SRCS:.c=.o)
TARGET = ls

.PHONY: all clean test

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET) $(LDFLAGS)

src/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f src/*.o $(TARGET)

test: $(TARGET)
	chmod +x tests/run_tests.sh
	./tests/run_tests.sh
