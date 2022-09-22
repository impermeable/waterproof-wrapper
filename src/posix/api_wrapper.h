// This file is part of WaterProof.
// Copyright (C) 2019  The ChefCoq team.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

#ifndef WPWRAPPER_API_WRAPPER_H
#define WPWRAPPER_API_WRAPPER_H

#include "api.h"

namespace wpwrapper {

class api_wrapper : public api {
public:

    int accept(int sockfd, struct sockaddr* addr, socklen_t* addrlen) const noexcept override;

    int bind(int sockfd, const struct sockaddr* addr, socklen_t addrlen) const noexcept override;

    void _exit(int status) const noexcept override;

    int close(int fd) const noexcept override;

    int dup2(int oldfd, int newfd) const noexcept override;

    int execv(const char* path, char* const argv[]) const noexcept override;

    int fcntl(int fd, int cmd, int opt) const noexcept override;

    pid_t fork() const noexcept override;

    void freeaddrinfo(struct addrinfo* res) const noexcept override;

    int getaddrinfo(const char* node, const char* service, const struct addrinfo* hints,
            struct addrinfo** res) const noexcept override;

    int ioctl(int fd, unsigned long request, void* argp) const noexcept override;

    int listen(int sockfd, int backlog) const noexcept override;

    int kill(pid_t pid, int sig) const noexcept override;

    int pipe(int pipefd[2]) const noexcept override;

    int poll(struct pollfd* fds, nfds_t nfds, int timeout) const noexcept override;

    ssize_t read(int fd, void* buf, size_t count) const noexcept override;

    ssize_t recv(int sockfd, void* buf, size_t len, int flags) const noexcept override;

    int select(int nfds, fd_set* readfds, fd_set* writefds, fd_set* exceptfds,
            struct timeval* timeout) const noexcept override;

    ssize_t send(int sockfd, const void* buf, size_t len, int flags) const noexcept override;

    int setsockopt(int sockfd, int level, int optname, const void* optval, socklen_t optlen) const noexcept override;

    int shutdown(int sockfd, int how) const noexcept override;

    int socket(int domain, int type, int protocol) const noexcept override;

    pid_t waitpid(pid_t pid, int* wstatus, int options) const noexcept override;

    ssize_t write(int fd, const void* buf, size_t count) const noexcept override;
};

} // namespace wpwrapper

#endif // WPWRAPPER_API_WRAPPER_H
