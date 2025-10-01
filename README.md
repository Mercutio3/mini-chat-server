# Mini Chat Server - C++

**Version 1.1.1**

This repo contains a pair of simple but professional multi-client chat servers written in C and C++.

The C server, intially a simple echoing program with support for multiple concurrent clients only via forking, was rewritten to utilize the `kqueue` library. This unfortunately limited server compatibility to macOS.

To increase portability, I decided to write a C++ version of the server with identical functionality, instead using multithreading to handle multiple clients. Right now development is focused on this C++ server, contained in the `main-cpp` branch. For the C version, see the `main` branch.

The goal of this project is to demonstrate my understanding of the skills in networking, OOP, multithreading, and event-driven programming aquired during my low-level programming courses, as well as the design, DevOps, and production-minded qualities upheld in my software engineering courses. However, this server is **NOT** secure for production; it is simply a small open-source demo. Please see Limitations section for more.

## Features

- Multithreading C++ server
- Portable clients
- Multi-client support
- Usernames, and the ability to change them
- Broadcast and private messaging
- List of connected users
- Graceful server shutdown via signal handling
- Robust handling of errors, disconnects, resource exhaustion, and invalid user input
- Modular codebase
- Clean socket + resource management
- Chat logging

## Dependencies

To run this you will need:

- A C++17 compiler like g++ or clang++ 
- make
- cmake (for building GoogleTest)
- GoogleTest (added as git submodule)
- AddressSanitizer (optional, for memory testing)

On macOS, Xcode Command Line Tools include g++, clang++, and make. If you don't have them already, simply run `xcode-select --install` on a terminal window.

## Installation Guide

1. Clone repository and initialize submodules:

    git clone https://github.com/yourusername/yourrepo.git
    cd yourrepo
    git submodule update --init --recursive

2. On a terminal window in the cloned repo's root, build GoogleTest. (Done automatically by Makefile or CI, but you can do this manually if needed)

    cd third_party/googletest
    cmake -S . -B build
    cmake --build build
    cd ../..

3. Run "`make`" to build the project:

    make

## Launching

On a terminal window you wish to use as a server (opened on the cloned repo's root directory), run ./tserver <port>. The server takes a single argument for the desired port number, which can range from `1024` to `65535`. Afterwards, it an only be shut down with CTRL+C.

On a terminal window you wish to use as a client (also opened on the cloned repo's root directory), run ./client <ip> <port>. The client takes two argument, the IP address of the server and the server port. The program was built using localhost, or `127.0.0.1` (`::1` in IPv6).

The default port for both the server and the client is `5223`.

## Running

From this point on any non-command message typed and sent in a client will be broadcasted to all other connected clients. A received message will be prefaced with the sending user's username, with the format "[ SENDER USERNAME ]: [ MESSAGE ]".

Clients can send commands to the server (prefaced by a backslash /) to execute specific functionalities. Some commands take arguments. Here is an overview:

- /help - Server will send a list of available commands to the requesting client.

- /exit - Server will close its connection to the requesting client, who will terminate gracefully.

- /list - Server will send a list of all connected clients' usernames to the requesting client.

- /name <newusername> - Server will change a client's username. Usernames must be unique, non-empty, have a max length, and cannot contain spaces/control characters. Invalid or duplicate usernames will be rejected. All future broadcasted and private messages will reflect a successful change.

- /msg <target> <message> - Server will forward a private message to the specfied target client. It will not work if either argument is missing, or if the target username does not exist. Clients receiving a private message will be notified of the sender.

## Docker Compose Usage

You can also easily run the server and multiple clients using Docker Compose. It will also take care of the networking; both the clients and server communicate over an internal Docker network called `chat-net`.

To build and start the server and one client:

    docker compose up --build

Alternatively, to run multiple clients, use the `--scale` flag:

    docker compose up --scale client=3

To interact with a specific client or server:

1. Open a new terminal window

2. See all running containers:

    docker ps

3. Attach to any of the available containers:

    docker attach chat-client-1

To stop and remove all containers and the network:

    docker compose down

Clients use the server's container name (`chat-server`) to connect to it, as well as the defualt port `5223`. To change the port, edit `docker-compose.yml` to pass different values in the `command` section.

## Logging

All broadcasts, private messages, connections, disconnections, and username changes are logged into `server.log`.
- Client logs are prefixed with their username, or "unknown" if the client has not set their username.
- Clients log into their console as well.
- Log file logs include a timestamp in UTC.
- If the log file already exists, the server will append instead of overwriting.

## Documentation

The codebase contains Doxygen-style comments for documentation. If you have Doxyfile configured, you can generate HTML documentation with:

    doxygen Doxyfile

### Optional Features

These can be enabled by adding a flag to line 2 of the Makefile:

    CXXFLAGS = -Wall -Wextra -Werror -g -Iinclude

The following flags are available:

- -DDEBUG: enables debug output, more on that in its respective section.
- -DRECEIVE_OWN_MESSAGE: Server sends broadcast message back to sender as well.
- -DINSTRUCTIONS: Print usage instructions for client every time a message is sent/received.
- -DLOCAL_EXIT: Client terminates immediately when using /exit instead of waiting for server to close connection.

Make sure to run `make clean && make` to re-compile everything.

## Testing

Unit tests are written using GoogleTest, a submodule that must be initialized. Run all tests with `make test`. The call will fail if any test fails.

Testing-related files can be found in the `tests` directory. Executables for each are created upon installation, which you can run individually:

    ./testClientList runs unit-tests on the frequently-called methods supporting the linked list that keeps track of all connected clients. 

    ./testCommands runs tests on the command-processing methods with both valid and invalid sets of arguments, as well as verifying their results.

    ./testUtils runs tests on helper functions used by other files.

Due to Valgrind not being available on macOS, ASan and LSan are used to detect memory leaks and buffer overflows. To enable them, simply change line 2 of the Makefile from:

    CXXFLAGS = -Wall -Wextra -Werror -g -Iinclude 

to

    CXXFLAGS = -Wall -Wextra -Werror -g -Iinclude -fsanitize=address

then run `make clean && make` to recompile everything.

## Limitations

- Only plain text messages supported
- Unencrypted communication
- No spam protection
- No user registration/passwords
- No GUI client
- May not scale to hundreds/thousands of clients
- Only supports UTF-8 (input is validated)

## Debugging

You can enable debug logging to print additional output and setup logs. Simply change line 2 of the Makefile from:

    CXXFLAGS = -Wall -Wextra -Werror -g -Iinclude

to

    CXXFLAGS = -Wall -Wextra -Werror -g -Iinclude -DDEBUG

and run `make clean && make` to re-compile everything.

## Version

The current version is defined in `include/version.hpp` as `CHAT_APP_VERSION`.

## C Version

As mentioned above, please see the `main` branch for the C server. The C server only works on macOS due to its use of `kqueue` for event handling.