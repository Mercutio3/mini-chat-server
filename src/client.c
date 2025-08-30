#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define MAXDATASIZE 200 // Max bytes that can be received at once

// #define DEBUG
// #define INSTRUCTIONS //Print instructions every time a message is
// sent/received #define LOCAL_EXIT //Client exits immediately when using /exit
// instead of waiting for server to close connection

void *get_in_addr(struct sockaddr *sa) {
  if (sa->sa_family == AF_INET) {
    return &(((struct sockaddr_in *)sa)->sin_addr);
  }
  return &(((struct sockaddr_in6 *)sa)->sin6_addr);
}

int main(int argc, char *argv[]) {
  int sockfd, numBytes;
  char buf[MAXDATASIZE];
  struct addrinfo hints, *servinfo, *p;
  int rv;
  char s[INET6_ADDRSTRLEN];

  char msg[MAXDATASIZE];

  // Print usage if no IP is provided
  if (argc != 3) {
    fprintf(stderr, "Usage: ./client <server ip/hostname> <port>\n");
    return EXIT_FAILURE;
  }

  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;

  if ((rv = getaddrinfo(argv[1], argv[2], &hints, &servinfo)) != 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
    return 1;
  }

  // Loop thorugh results and bind to first available address
  for (p = servinfo; p != NULL; p = p->ai_next) {
    if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
      perror("Client: socket");
      continue;
    }

    inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr), s,
              sizeof s);
#ifdef DEBUG
    printf("Attempting connection to %s\n", s);
#endif

    if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
      perror("Connecting error");
      close(sockfd);
      continue;
    }

    break;
  }

  if (p == NULL) {
    fprintf(stderr, "Failed to connect.\n");
    return 2;
  }

  inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr), s,
            sizeof(s));
  printf("Connected to: %s:%s\n", s, argv[2]);

  freeaddrinfo(servinfo); // Servinfo not used after here

  printf("----------------------------------------\n");
  printf("Write message for server, or '/help' for a list of commands:\n");

  while (1) {
    // Select() setup
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(STDIN_FILENO, &readfds);
    FD_SET(sockfd, &readfds);
    int maxfd = ((STDIN_FILENO > sockfd) ? STDIN_FILENO : sockfd) + 1;

    if (select(maxfd, &readfds, NULL, NULL, NULL) < 0) {
      perror("select error");
      return EXIT_FAILURE;
    }

    if (FD_ISSET(STDIN_FILENO, &readfds)) {
      // Read message from user input
      if (fgets(msg, sizeof(msg), stdin) == NULL) {
#ifdef DEBUG
        printf("EOF reached or error occurred. Exiting...\n");
#endif
        perror("fgets error or EOF reached");
        return EXIT_FAILURE;
      };
      msg[strcspn(msg, "\n")] = '\0';

      // Send message to server
      if (send(sockfd, msg, strlen(msg), 0) == -1) {
        perror("Error sending.");
      }

#ifdef LOCAL_EXIT
      // Close socket if user enters "exit"
      if (strcmp(msg, "/exit") == 0) {
        printf("Exiting...\n");
        break;
      }
#endif
      printf("Sent: %s\n", msg);
    }

    if (FD_ISSET(sockfd, &readfds)) {
      // Receive message from server
      if ((numBytes = recv(sockfd, buf, MAXDATASIZE - 1, 0)) == -1) {
        perror("recv error.");
        return EXIT_FAILURE;
      }

      // Client either disconneted or error occurred
      if (numBytes <= 0) {
        if (numBytes == 0) {
          // Disconnected
          printf("Server closed connection.\n");
        } else {
          // Error
          perror("recv error");
        }

        // In either case, close socket
        break;
      } else {
        buf[numBytes] = '\0';
        // Close socket if server sent shutdown instruction
        if (strncmp(buf, "SERVER_SHUTDOWN", 15) == 0) {
          printf("Server has closed. Exiting...\n");
          break;
        } else {
          printf("%s\n", buf);
#ifdef INSTRUCTIONS
          printf("----------------------------------------\n");
          printf("Write message for server, or 'exit' to exit:\n");
#endif
        }
      }
    }
  }
  close(sockfd);
  return EXIT_SUCCESS;
}