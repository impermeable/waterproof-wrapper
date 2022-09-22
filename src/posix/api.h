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

#ifndef WPWRAPPER_API_H
#define WPWRAPPER_API_H

#include <poll.h>
#include <netdb.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>

namespace wpwrapper {

class api {
public:

    virtual ~api() = default;

    /// \see https://manpages.ubuntu.com/manpages/disco/en/man2/accept.2.html
    virtual int accept(int sockfd, struct sockaddr* addr, socklen_t* addrlen) const noexcept = 0;

    /// \see https://manpages.ubuntu.com/manpages/disco/en/man2/bind.2.html
    virtual int bind(int sockfd, const struct sockaddr* addr, socklen_t addrlen) const noexcept = 0;

    /// \see https://manpages.ubuntu.com/manpages/disco/en/man2/exit.2.html
    virtual void _exit(int status) const noexcept = 0;

    /// \see https://manpages.ubuntu.com/manpages/disco/en/man2/close.2.html
    virtual int close(int fd) const noexcept = 0;

    /// \see https://manpages.ubuntu.com/manpages/disco/en/man2/dup2.2.html
    virtual int dup2(int oldfd, int newfd) const noexcept = 0;

    /// \see https://manpages.ubuntu.com/manpages/disco/en/man3/exec.3posix.html
    virtual int execv(const char* path, char* const argv[]) const noexcept = 0;

    /// \see http://manpages.ubuntu.com/manpages/disco/man2/fcntl.2.html
    virtual int fcntl(int fd, int cmd, int opt) const noexcept = 0;

    /// \see https://manpages.ubuntu.com/manpages/disco/en/man2/ioctl.2.html
    virtual int ioctl(int fd, unsigned long request, void* argp) const noexcept = 0;

    /// \see https://manpages.ubuntu.com/manpages/disco/en/man2/listen.2.html
    virtual int listen(int sockfd, int backlog) const noexcept = 0;

    /// \see https://manpages.ubuntu.com/manpages/disco/en/man2/fork.2.html
    virtual pid_t fork() const noexcept = 0;

    /// \see https://manpages.ubuntu.com/manpages/disco/en/man3/freeaddrinfo.3.html
    virtual void freeaddrinfo(struct addrinfo* res) const noexcept = 0;

    /// \see https://manpages.ubuntu.com/manpages/disco/en/man3/freeaddrinfo.3.html
    virtual int getaddrinfo(const char* node, const char* service, const struct addrinfo* hints,
            struct addrinfo** res) const noexcept = 0;

    /// \see https://manpages.ubuntu.com/manpages/disco/en/man2/kill.2.html
    virtual int kill(pid_t pid, int sig) const noexcept = 0;

    /// \see https://manpages.ubuntu.com/manpages/disco/en/man2/pipe.2.html
    virtual int pipe(int pipefd[2]) const noexcept = 0;

    /// \see https://manpages.ubuntu.com/manpages/disco/en/man2/poll.2.html
    virtual int poll(struct pollfd* fds, nfds_t nfds, int timeout) const noexcept = 0;

    /// \see https://manpages.ubuntu.com/manpages/disco/en/man2/read.2.html
    virtual ssize_t read(int fd, void* buf, size_t count) const noexcept = 0;

    /// \see https://manpages.ubuntu.com/manpages/disco/en/man2/recv.2.html
    virtual ssize_t recv(int sockfd, void* buf, size_t len, int flags) const noexcept = 0;

    /// \see https://manpages.ubuntu.com/manpages/disco/en/man2/select.2.html
    virtual int
    select(int nfds, fd_set* readfds, fd_set* writefds, fd_set* exceptfds, struct timeval* timeout) const noexcept = 0;

    /// \see https://manpages.ubuntu.com/manpages/disco/en/man2/send.2.html
    virtual ssize_t send(int sockfd, const void* buf, size_t len, int flags) const noexcept = 0;

    /// \see https://manpages.ubuntu.com/manpages/disco/en/man2/setsockopt.2.html
    virtual int setsockopt(int sockfd, int level, int optname, const void* optval, socklen_t optlen) const noexcept = 0;

    /// \see https://manpages.ubuntu.com/manpages/disco/en/man2/shutdown.2.html
    virtual int shutdown(int sockfd, int how) const noexcept = 0;

    /// \see https://manpages.ubuntu.com/manpages/disco/en/man2/socket.2.html
    virtual int socket(int domain, int type, int protocol) const noexcept = 0;

    /// \see https://manpages.ubuntu.com/manpages/disco/en/man2/waitpid.2.html
    virtual pid_t waitpid(pid_t pid, int* wstatus, int options) const noexcept = 0;

    /// \see https://manpages.ubuntu.com/manpages/disco/en/man2/write.2.html
    virtual ssize_t write(int fd, const void* buf, size_t count) const noexcept = 0;
};

} // namespace wpwrapper

#endif // WPWRAPPER_API_H
