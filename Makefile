CC = gcc
CFLAGS = -Wall -Wextra -Werror -g -Iinclude 

SERVER_OBJS = src/kserver.o src/clientList.o src/commands.o src/log.o
SERVER_TARGET = kserver

CLIENT_OBJS = src/client.o
CLIENT_TARGET = client

TEST_CLIENTLIST_SRC = tests/testClientList.c src/clientList.c src/log.c
TEST_COMMANDS_SRC = tests/testCommands.c src/commands.c src/clientList.c src/log.c

TEST_CLIENTLIST = testClientList
TEST_COMMANDS = testCommands

all: $(SERVER_TARGET) $(CLIENT_TARGET) $(TEST_CLIENTLIST) $(TEST_COMMANDS)

$(SERVER_TARGET): $(SERVER_OBJS)
	$(CC) $(CFLAGS) -o $@ $(SERVER_OBJS)

$(CLIENT_TARGET): $(CLIENT_OBJS)
	$(CC) $(CFLAGS) -o $@ $(CLIENT_OBJS)

$(TEST_CLIENTLIST): $(TEST_CLIENTLIST_SRC)
	$(CC) $(CFLAGS) -o $@ $(TEST_CLIENTLIST_SRC)

$(TEST_COMMANDS): $(TEST_COMMANDS_SRC)
	$(CC) $(CFLAGS) -DMOCK_SEND -o $@ $(TEST_COMMANDS_SRC)

src/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

test: $(TEST_CLIENTLIST) $(TEST_COMMANDS)
	./$(TEST_CLIENTLIST)
	./$(TEST_COMMANDS)

clean:
	$(RM) *.o src/*.o $(SERVER_TARGET) $(CLIENT_TARGET) $(TEST_CLIENTLIST) $(TEST_COMMANDS)

.PHONY: all clean