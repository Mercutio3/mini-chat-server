CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -Werror -g -Iinclude

GTEST_INC = third_party/googletest/googletest/include
GTEST_LIB = third_party/googletest/lib

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
	$(CXX) $(CXXFLAGS) -I$(GTEST_INC) -DMOCK_SEND -o $@ $(TEST_CLIENTLIST_SRC) -L$(GTEST_LIB) -lgtest -lgtest_main -lpthread

$(TEST_COMMANDS): $(TEST_COMMANDS_SRC)
	$(CXX) $(CXXFLAGS) -I$(GTEST_INC) -DMOCK_SEND -o $@ $(TEST_COMMANDS_SRC) -L$(GTEST_LIB) -lgtest -lgtest_main -lpthread

$(TEST_UTILS): $(TEST_UTILS_SRC)
	$(CXX) $(CXXFLAGS) -I$(GTEST_INC) -o $@ $(TEST_UTILS_SRC) -L$(GTEST_LIB) -lgtest -lgtest_main -lpthread

src/%.o: src/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

test: $(TEST_CLIENTLIST) $(TEST_COMMANDS) $(TEST_UTILS)
	./$(TEST_CLIENTLIST)
	./$(TEST_COMMANDS)
	./$(TEST_UTILS)

clean:
	$(RM) *.o src/*.o $(SERVER_TARGET) $(CLIENT_TARGET) $(TEST_CLIENTLIST) $(TEST_COMMANDS) $(TEST_UTILS)

.PHONY: all clean