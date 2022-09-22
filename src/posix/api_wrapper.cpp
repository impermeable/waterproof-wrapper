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

#include "api_wrapper.h"

namespace wpwrapper {

int api_wrapper::accept(int sockfd, struct sockaddr* addr, socklen_t* addrlen) const noexcept
{
    return ::accept(sockfd, addr, addrlen);
}

int api_wrapper::bind(int sockfd, const struct sockaddr* addr, socklen_t addrlen) const noexcept
{
    return ::bind(sockfd, addr, addrlen);
}

void api_wrapper::_exit(int status) const noexcept
{
    ::_exit(status);
}

int api_wrapper::close(int fd) const noexcept
{
    return ::close(fd);
}

int api_wrapper::dup2(int oldfd, int newfd) const noexcept
{
    return ::dup2(oldfd, newfd);
}

int api_wrapper::execv(const char* path, char* const argv[]) const noexcept
{
    return ::execv(path, argv);
}

int api_wrapper::fcntl(int fd, int cmd, int opt) const noexcept
{
    return ::fcntl(fd, cmd, opt);
}

pid_t api_wrapper::fork() const noexcept
{
    return ::fork();
}

void api_wrapper::freeaddrinfo(struct addrinfo* res) const noexcept
{
    ::freeaddrinfo(res);
}

int api_wrapper::getaddrinfo(const char* node, const char* service, const struct addrinfo* hints,
        struct addrinfo** res) const noexcept
{
    return ::getaddrinfo(node, service, hints, res);
}

int api_wrapper::ioctl(int fd, unsigned long request, void* argp) const noexcept
{
    return ::ioctl(fd, request, argp);
}

int api_wrapper::listen(int sockfd, int backlog) const noexcept
{
    return ::listen(sockfd, backlog);
}

int api_wrapper::kill(pid_t pid, int sig) const noexcept
{
    return ::kill(pid, sig);
}

int api_wrapper::pipe(int pipefd[2]) const noexcept
{
    return ::pipe(pipefd);
}

int api_wrapper::poll(struct pollfd* fds, nfds_t nfds, int timeout) const noexcept
{
    return ::poll(fds, nfds, timeout);
}

ssize_t api_wrapper::read(int fd, void* buf, size_t count) const noexcept
{
    return ::read(fd, buf, count);
}

ssize_t api_wrapper::recv(int sockfd, void* buf, size_t len, int flags) const noexcept
{
    return ::recv(sockfd, buf, len, flags);
}

int api_wrapper::select(int nfds, fd_set* readfds, fd_set* writefds, fd_set* exceptfds,
        struct timeval* timeout) const noexcept
{
    return ::select(nfds, readfds, writefds, exceptfds, timeout);
}

ssize_t api_wrapper::send(int sockfd, const void* buf, size_t len, int flags) const noexcept
{
    return ::send(sockfd, buf, len, flags);
}

int api_wrapper::setsockopt(int sockfd, int level, int optname, const void* optval, socklen_t optlen) const noexcept
{
    return ::setsockopt(sockfd, level, optname, optval, optlen);
}

int api_wrapper::shutdown(int sockfd, int how) const noexcept
{
    return ::shutdown(sockfd, how);
}

int api_wrapper::socket(int domain, int type, int protocol) const noexcept
{
    return ::socket(domain, type, protocol);
}

pid_t api_wrapper::waitpid(pid_t pid, int* wstatus, int options) const noexcept
{
    return ::waitpid(pid, wstatus, options);
}

ssize_t api_wrapper::write(int fd, const void* buf, size_t count) const noexcept
{
    return ::write(fd, buf, count);
}

} // namespace wpwrapper
