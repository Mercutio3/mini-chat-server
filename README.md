# Mini Chat Servers

A pair of simple but professional multi-client chat servers written in C and C++.

The C server started off as a simple echoing program, with support for multiple concurrent clients only via forking. To better handle more clients and to favor performant scalability, the server was rewritten to utilize the kqueue library. This unfortunately limits server compatibility to MacOS.

After the C server reached an acceptable release state, I decided to write a C++ implmentation with the same funcitonality, but using multithreading to handle multiple clients. The C project is contained in main, while the C++ project is in the "cpp-multithreading" branch.

The goal of this project is to demonstrate my understanding of the skills in networking, OOP, multithreading, and event-driven programming aquired during my low-level programming courses, as well as the design, DevOps, and production-minded qualities upheld in my software engineering courses.

However, this server is NOT secure for production; it is simply a small open-source demo. Please see Limitations section for more.

## Features

    - Event-driven C server
    - Multithreading C++ server
    - Portable clients
    - Multi-client support
    - Usernames, and the ability to change them
    - Broadcast and private messaging
    - List of connected users
    - Graceful shutdown via signal handling
    - Robust handling of errors, disconnects, resource exhaustion, and invalid user input
    - Modular codebase
    - Clean socket + resource management
    - Chat logging

## Dependencies

To run this you will need:

    - A C compiler like gcc or clang, or a C++ compiler like g++
    - make
    - kqueue
    - AddressSanitizer (optional, for memory testing)

On macOS, Xcode Command Line Tools include gcc, clang, make, and kqueue. So if you don't have it already, simply run:

    xcode-select --install

on a terminal window.

## Installation Guide

    1. Download the source code from a given release, unzip it.
    
    2. Open a terminal window in the cloned repo's root directory, or cd to it
    
    3. Run "make".

## Launching

### C Version

On a terminal window you wish to use as a server (opened on the cloned repo's root directory), run ./kserver. The server takes no arguments, and can only be interacted with to shut down (CTRL + C).

On a terminal window you wish to use as a client (also opened on the cloned repo's root directory), run ./client [ ip ]. The client takes one argument, the IP address of the server. The program was built using localhost, or 127.0.0.1 (::1 in IPv6).

The default port for both the server and the client is 5223.

### C++ Version

Instead of ./kserver, run ./tserver.

## Running

From this point on any non-command message typed and sent in a client will be broadcasted to all other connected clients. A received message will be prefaced with the sending user's username, with the format "[ SENDER USERNAME ]: [ MESSAGE ]".

Clients can send commands to the server (prefaced by a backslash /) to execute specific functionalities. Some commands take arguments. Here is an overview:

    # /help - Server will send a list of available commands to the requesting client who requested it.

    # /exit - Server will close its connection to the requesting client, who will termiante gracefully.

    # /list - Server will send a list of all connected clients' usernames to the requesting client.

    # /name [new username] - Server will change a client's username. Usernames have a max length, cannot contain spaces, and cannot be NULL. If new username is valid, all future broadcasted and private messages will reflect this change.

    # /msg [target username] [message] - Server will forward a private message to the specfied target client. It will not work if either of the arguments are NULL, or if the target username does not exist. Clients receiving a private message will be notified of who the sender is.

A log file "chatLog.log" (or "server.log" for the C++ server) will be created in the root directory. The following things will be logged here:

    1. Broadcast chat messages
    2. Private chat messages (denoted as "sender -> receiver: ")
    3. Server startup
    4. Server shutdown
    5. Clients connecting and disconnecting
    6. Succesful username changes

These logs include a timestamp in UTC. If the log file already exists, the server will append instead of overwriting.

### Optional Features

These can be enabled by adding a flag to line 2 of the Makefile:

    CFLAGS = -Wall -Wextra -Werror -g -Iinclude

The following flags are available:

    -DDEBUG: enables debug output, more on that in its respective section
    -DRECEIVE_OWN_MESSAGE: Server sends broadcast message back to sender as well
    -DINSTRUCTIONS: Print usage instructions for client every time a message is sent/received
    -DLOCAL_EXIT: Client terminates immediately when using /exit instead of waiting for server to close connection

Make sure to run "make clean && make" to re-compile everything.

## Testing

All testing-related files can be found in the tests directory. Executables for each are created upon installation, which you can run individually:

    ./testClientList runs unit-tests on the frequently-called methods supporting the linked list that keeps track of all connected clients. 

    ./testCommands runs tests on the command-processing methods with both valid and invalid sets of arguments, as well as verifying their results.

    ./testUtils runs tests on helper functions used by other files.

Alternatively, run "make test" to build and run tests. The call will fail if any test fails.

Due to Valgrind not being available on macOS, ASan and LSan are used to detect memory leaks and buffer overflows. To enable them, simply change line 2 of the Makefile from:

    CFLAGS = -Wall -Wextra -Werror -g -Iinclude 

to

    CFLAGS = -Wall -Wextra -Werror -g -Iinclude -fsanitize=address

then run "make clean && make" to recompile everything.

## Limitations

    - As mentioned above, due to the kqueue interface being a macOS implementation of Linux's epoll, the server only runs on macOS.
    - Only plain text messages supported
    - Unencrypted communication
    - No spam protection
    - No user registration/passwords
    - No GUI client
    - May not scale to hundreds/thousands of clients
    - Only supports UTF-8 (input is verified)

## Debugging

You can enable debug logging to print additional output at all times. Simply change line 2 of the Makefile from:

    CFLAGS = -Wall -Wextra -Werror -g -Iinclude

to

    CFLAGS = -Wall -Wextra -Werror -g -Iinclude -DDEBUG

and run "make clean && make" to re-compile everything.