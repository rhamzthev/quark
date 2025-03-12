CC = gcc
CFLAGS = -Wall -Wextra -I./src
LDFLAGS = -lncurses -lm

SRCS = ./src/main.c ./src/explorer/explorer.c ./src/editor/editor.c
OBJS = $(SRCS:.c=.o)
TARGET = ./dist/quark

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: clean run

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(OBJS) $(TARGET)