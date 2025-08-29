CC = gcc
CFLAGS = -Wall -Wextra -g

SERVER_OBJS = src/kserver.o src/clientList.o src/commands.o
SERVER_TARGET = kserver

CLIENT_OBJS = src/client.o
CLIENT_TARGET = client

TEST_CLIENTLIST_SRC = tests/testClientList.c src/clientList.c
TEST_COMMANDS_SRC = tests/testCommands.c src/commands.c src/clientList.c

TEST_CLIENTLIST = testClientList
TEST_COMMANDS = testCommands

CFLAGS += -Iinclude

all: $(SERVER_TARGET) $(CLIENT_TARGET) $(TEST_CLIENTLIST) $(TEST_COMMANDS)

$(SERVER_TARGET): $(SERVER_OBJS)
	$(CC) $(CFLAGS) -o $@ $(SERVER_OBJS)

$(CLIENT_TARGET): $(CLIENT_OBJS)
	$(CC) $(CFLAGS) -o $@ $(CLIENT_OBJS)

$(TEST_CLIENTLIST): $(TEST_CLIENTLIST_SRC)
	$(CC) $(CFLAGS) -o $@ $(TEST_CLIENTLIST_SRC)

$(TEST_COMMANDS): $(TEST_COMMANDS_SRC)
	$(CC) $(CFLAGS) -o $@ $(TEST_COMMANDS_SRC)

src/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f *.o src/*.o $(SERVER_TARGET) $(CLIENT_TARGET) $(TEST_CLIENTLIST) $(TEST_COMMANDS)

.PHONY: all clean