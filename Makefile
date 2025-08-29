CC = gcc
CFLAGS = -Wall -Wextra -g

SERVER_OBJS = src/kserver.o src/clientList.o src/commands.o
SERVER_TARGET = kserver

CLIENT_OBJS = src/client.o
CLIENT_TARGET = client

all: $(SERVER_TARGET) $(CLIENT_TARGET)

$(SERVER_TARGET): $(SERVER_OBJS)
	$(CC) $(CFLAGS) -o $@ $(SERVER_OBJS)

$(CLIENT_TARGET): $(CLIENT_OBJS)
	$(CC) $(CFLAGS) -o $@ $(CLIENT_OBJS)

src/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f src/*.o $(SERVER_TARGET) $(CLIENT_TARGET)

.PHONY: all clean