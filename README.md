# Mini Chat Server

A simple but professional multi-client chat server written in C.

The server started off as a simple echoing program, with support for multiple concurrent clients only via forking. To better handle more clients and to favor performant scalability, the server was rewritten to utilize the kqueue library. This unfortunately limits server compatibility to MacOS.

The goal of this project is to demonstrate and put to practice the skills in networking, concurrency, and event-driven programming aquired during my low-level programming courses, as well as the design, DevOps, and production-minded qualities upheld in my software engineering courses.

## Features

    - Event-driven server
    - Portable client
    - Multi-client support
    - Usernames, and the ability to change them
    - Broadcast and private messaging
    - List of connected users
    - Graceful shutdown via signal handling
    - Robust handling of errors, disconnects, resource exhaustion, and invalid user input
    - Modular codebase
    - Clean socket + resource management

## Dependencies

To run this you will need:

    - A C compiler like gcc or clang
    - make
    - kqueue
    - AddressSanitizer (optional, for memory testing)

On macOS, Xcode Command Line Tools include gcc, clang, make, and kqueue. So if you don't have it already, simply run:

    xcode-select --install

on a terminal window.

## Installation Guide

    1. Clone this repository
        git clone https://github.com/Mercutio3/mini-chat-server.git
    
    2. Open a terminal window in the cloned repo's root directory, or cd to it
    
    3. Run "make".

## Running

On a terminal window you wish to use as a server (opened on the cloned repo's root directory), run ./kserver. The server takes no arguments, and can only be interacted with to shut down (CTRL + C).

On a terminal window you wish to use as a client (also opened on the cloned repo's root directory), run ./client [ ip ]. The client takes one argument, the IPv4 address of the server. The program was built using localhost, or 127.0.0.1.

Both server and client currently use the hard-coded port 5223.

From this point on any non-command message typed and sent in a client will be broadcasted to all other connected clients. A received message will be prefaced with the sending user's username, with the format "[ SENDER USERNAME ]: [ MESSAGE ]".

Clients can send commands to the server (prefaced by a backslash /) to execute specific functionalities. Some commands take arguments. Here is an overview:

    # /help - Server will send a list of available commands to the requesting client who requested it.

    # /exit - Server will close its connection to the requesting client, who will termiante gracefully.

    # /list - Server will send a list of all connected clients' usernames to the requesting client.

    # /name [new username] - Server will change a client's username. Usernames have a max length, cannot contain spaces, and cannot be NULL. If new username is valid, all future broadcasted and private messages will reflect this change.

    # /msg [target username] [message] - Server will forward a private message to the specfied target client. It will not work if either of the arguments are NULL, or if the target username does not exist. Clients receiving a private message will be notified of who the sender is.

## Testing

All testing-related files can be found in the tests directory. Executables for each are created upon installation, which you can run individually:

    ./testClientList runs unit-tests on the frequently-called methods supporting the linked list that keeps track of all connected clients. 

    ./testCommands runs tests on the command-processing methods with both valid and invalid sets of arguments, as well as verifying their results.

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
    - Messages do not persist after server shutdown
    - No spam protection
    - No user registration/passwords
    - No GUI client
    - May not scale to hundreds/thousands of clients
    - Assumption of UTF-8 or ASCII
    - No IPv6 support

## Debugging

You can enable debug logging to print additional output at all times. Simply change line 2 of the Makefile from:

    CFLAGS = -Wall -Wextra -Werror -g -Iinclude

to

    CFLAGS = -Wall -Wextra -Werror -g -Iinclude -DDEBUG

and run "make clean && make" to re-compile everything.