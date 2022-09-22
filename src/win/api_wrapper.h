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

    SOCKET accept(SOCKET s, sockaddr* addr, int* addrlen) const noexcept override;

    int bind(SOCKET s, const sockaddr* name, int namelen) const noexcept override;

    BOOL CancelIoEx(HANDLE hFile, LPOVERLAPPED lpOverlapped) const noexcept override;

    BOOL CloseHandle(HANDLE hObject) const noexcept override;

    int closesocket(SOCKET s) const noexcept override;

    HANDLE CreateEventA(LPSECURITY_ATTRIBUTES lpEventAttributes, BOOL bManualReset, BOOL bInitialState,
            LPCSTR lpName) const noexcept override;

    HANDLE
    CreateFileA(LPCSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes,
            DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile) const noexcept override;

    HANDLE
    CreateNamedPipeA(LPCSTR lpName, DWORD dwOpenMode, DWORD dwPipeMode, DWORD nMaxInstances, DWORD nOutBufferSize,
            DWORD nInBufferSize, DWORD nDefaultTimeOut,
            LPSECURITY_ATTRIBUTES lpSecurityAttributes) const noexcept override;

    BOOL CreateProcessA(LPCSTR lpApplicationName, LPSTR lpCommandLine, LPSECURITY_ATTRIBUTES lpProcessAttributes,
            LPSECURITY_ATTRIBUTES lpThreadAttributes, BOOL bInheritHandles, DWORD dwCreationFlags, LPVOID lpEnvironment,
            LPCSTR lpCurrentDirectory, LPSTARTUPINFOA lpStartupInfo,
            LPPROCESS_INFORMATION lpProcessInformation) const noexcept override;

    VOID freeaddrinfo(PADDRINFOA pAddrInfo) const noexcept override;

    INT getaddrinfo(PCSTR pNodeName, PCSTR pServiceName, const ADDRINFOA* pHints,
            PADDRINFOA* ppResult) const noexcept override;

    DWORD GetLastError() const noexcept override;

    BOOL GetOverlappedResult(HANDLE hFile, LPOVERLAPPED lpOverlapped, LPDWORD lpNumberOfBytesTransferred,
            BOOL bWait) const noexcept override;

    int listen(SOCKET s, int backlog) const noexcept override;

    BOOL ReadFile(HANDLE hFile, LPVOID lpBuffer, DWORD nNumberOfBytesToRead, LPDWORD lpNumberOfBytesRead,
            LPOVERLAPPED lpOverlapped) const noexcept override;

    int recv(SOCKET s, char* buf, int len, int flags) const noexcept override;

    BOOL ResetEvent(HANDLE hEvent) const noexcept override;

    int send(SOCKET s, const char* buf, int len, int flags) const noexcept override;

    int
    select(int nfds, fd_set* readfds, fd_set* writefds, fd_set* exceptfds, PTIMEVAL timeout) const noexcept override;

    BOOL SetEvent(HANDLE hEvent) const noexcept override;

    int shutdown(SOCKET s, int how) const noexcept override;

    SOCKET socket(int af, int type, int protocol) const noexcept override;

    BOOL TerminateProcess(HANDLE hProcess, UINT uExitCode) const noexcept override;

    DWORD WaitForSingleObject(HANDLE hHandle, DWORD dwMilliseconds) const noexcept override;

    DWORD WaitForMultipleObjects(DWORD nCount, const HANDLE* lpHandles, BOOL bWaitAll,
            DWORD dwMilliseconds) const noexcept override;

    BOOL WriteFile(HANDLE hFile, LPVOID lpBuffer, DWORD nNumberOfBytesToWrite, LPDWORD lpNumberOfBytesWritten,
            LPOVERLAPPED lpOverlapped) const noexcept override;

    int WSACleanup() const noexcept override;

    int WSAGetLastError() const noexcept override;

    int WSAPoll(LPWSAPOLLFD fdArray, ULONG fds, INT timeout) const noexcept override;

    int WSAStartup(WORD wVersionRequired, LPWSADATA lpWSAData) const noexcept override;

};

} // namespace wpwrapper

#endif // WPWRAPPER_API_WRAPPER_H
