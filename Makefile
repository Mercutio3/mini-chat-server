CC = gcc
CFLAGS = -Wall -Wextra -g

SERVER_OBJS = kserver.o clientList.o commands.o
SERVER_TARGET = kserver

CLIENT_OBJS = client.o
CLIENT_TARGET = client

all: $(SERVER_TARGET) $(CLIENT_TARGET)

$(SERVER_TARGET): $(SERVER_OBJS)
	$(CC) $(CFLAGS) -o $@ $(SERVER_OBJS)

$(CLIENT_TARGET): $(CLIENT_OBJS)
	$(CC) $(CFLAGS) -o $@ $(CLIENT_OBJS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f *.o $(SERVER_TARGET) $(CLIENT_TARGET)

.PHONY: all clean