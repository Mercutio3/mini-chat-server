CC = gcc
CFLAGS = -Wall -Wextra -Werror -g -Iinclude

SERVER_OBJS = src/kserver.o src/clientList.o src/commands.o src/log.o src/utils.o
SERVER_TARGET = kserver

CLIENT_OBJS = src/client.o
CLIENT_TARGET = client

TEST_CLIENTLIST_SRC = tests/testClientList.c src/clientList.c src/log.c src/utils.c
TEST_COMMANDS_SRC = tests/testCommands.c src/commands.c src/clientList.c src/log.c src/utils.c
TEST_UTILS_SRC = tests/testUtils.c src/utils.c src/clientList.c

TEST_CLIENTLIST = testClientList
TEST_COMMANDS = testCommands
TEST_UTILS = testUtils

all: $(SERVER_TARGET) $(CLIENT_TARGET) $(TEST_CLIENTLIST) $(TEST_COMMANDS) $(TEST_UTILS)

$(SERVER_TARGET): $(SERVER_OBJS)
	$(CC) $(CFLAGS) -o $@ $(SERVER_OBJS)

$(CLIENT_TARGET): $(CLIENT_OBJS)
	$(CC) $(CFLAGS) -o $@ $(CLIENT_OBJS)

$(TEST_CLIENTLIST): $(TEST_CLIENTLIST_SRC)
	$(CC) $(CFLAGS) -o $@ $(TEST_CLIENTLIST_SRC)

$(TEST_COMMANDS): $(TEST_COMMANDS_SRC)
	$(CC) $(CFLAGS) -DMOCK_SEND -o $@ $(TEST_COMMANDS_SRC)

$(TEST_UTILS): $(TEST_UTILS_SRC)
	$(CC) $(CFLAGS) -o $@ $(TEST_UTILS_SRC)

src/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

test: $(TEST_CLIENTLIST) $(TEST_COMMANDS) $(TEST_UTILS)
	./$(TEST_CLIENTLIST)
	./$(TEST_COMMANDS)
	./$(TEST_UTILS)

clean:
	$(RM) *.o src/*.o $(SERVER_TARGET) $(CLIENT_TARGET) $(TEST_CLIENTLIST) $(TEST_COMMANDS) $(TEST_UTILS)

.PHONY: all clean