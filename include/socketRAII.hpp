/**
 * @file socketRAII.hpp
 * @brief RAII wrapper for socket file descriptors.
 */

#ifndef SOCKET_RAII_HPP
#define SOCKET_RAII_HPP

#include <unistd.h>

/**
 * @brief RAII wrapper for socket file descriptors.
 * 
 * Automatically closes the socket when the object goes out of scope.
 */
class SocketRAII {
    int fd;

  public:
    explicit SocketRAII(int fd = -1) : fd(fd) {}
    ~SocketRAII() {
        if (fd != -1) {
            close(fd);
        }
    }
    SocketRAII(const SocketRAII &) = delete;
    SocketRAII &operator=(const SocketRAII &) = delete;
    SocketRAII(SocketRAII &&other) noexcept : fd(other.fd) { other.fd = -1; }
    SocketRAII &operator=(SocketRAII &&other) noexcept {
        if (this != &other) {
            if (fd != -1) {
                close(fd);
            }
            fd = other.fd;
            other.fd = -1;
        }
        return *this;
    }
    int get() const { return fd; }
    operator int() const { return fd; }
};

#endif