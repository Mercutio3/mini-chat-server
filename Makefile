CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra -Werror -g -Iinclude

SERVER_OBJS = src/tserver.o src/clientList.o src/commands.o src/log.o
SERVER_TARGET = tserver

CLIENT_OBJS = src/client.o src/log.o
CLIENT_TARGET = client

TEST_CLIENTLIST_SRC = tests/testClientList.cpp src/clientList.cpp src/log.cpp
TEST_COMMANDS_SRC = tests/testCommands.cpp src/commands.cpp src/clientList.cpp src/log.cpp
TEST_UTILS_SRC = tests/testUtils.cpp

TEST_CLIENTLIST = testClientList
TEST_COMMANDS = testCommands
TEST_UTILS = testUtils

all: $(SERVER_TARGET) $(CLIENT_TARGET) $(TEST_CLIENTLIST) $(TEST_COMMANDS) $(TEST_UTILS)

$(SERVER_TARGET): $(SERVER_OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $(SERVER_OBJS)

$(CLIENT_TARGET): $(CLIENT_OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $(CLIENT_OBJS)

$(TEST_CLIENTLIST): $(TEST_CLIENTLIST_SRC)
	$(CXX) $(CXXFLAGS) -o $@ $(TEST_CLIENTLIST_SRC)

$(TEST_COMMANDS): $(TEST_COMMANDS_SRC)
	$(CXX) $(CXXFLAGS) -DMOCK_SEND -o $@ $(TEST_COMMANDS_SRC)

$(TEST_UTILS): $(TEST_UTILS_SRC)
	$(CXX) $(CXXFLAGS) -o $@ $(TEST_UTILS_SRC)

src/%.o: src/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

test: $(TEST_CLIENTLIST) $(TEST_COMMANDS) $(TEST_UTILS)
	./$(TEST_CLIENTLIST)
	./$(TEST_COMMANDS)
	./$(TEST_UTILS)

clean:
	$(RM) *.o src/*.o $(SERVER_TARGET) $(CLIENT_TARGET) $(TEST_CLIENTLIST) $(TEST_COMMANDS) $(TEST_UTILS)

.PHONY: all clean