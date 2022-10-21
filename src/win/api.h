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

#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>

namespace wpwrapper {

/// \brief Interface that wraps around various Win32 system calls.
class api {
public:

    virtual ~api() = default;

    /// \see https://docs.microsoft.com/en-us/windows/desktop/api/winsock2/nf-winsock2-accept
    virtual SOCKET accept(SOCKET s, sockaddr* addr, int* addrlen) const noexcept = 0;

    /// \see https://docs.microsoft.com/en-us/windows/desktop/api/winsock2/nf-winsock2-bind
    virtual int bind(SOCKET s, const sockaddr* name, int namelen) const noexcept = 0;

    /// \see https://docs.microsoft.com/en-us/windows/desktop/FileIO/cancelioex-func
    virtual BOOL CancelIoEx(HANDLE hFile, LPOVERLAPPED lpOverlapped) const noexcept = 0;

    /// \see https://docs.microsoft.com/en-us/windows/desktop/api/handleapi/nf-handleapi-closehandle
    virtual BOOL CloseHandle(HANDLE hObject) const noexcept = 0;

    /// \see https://docs.microsoft.com/en-us/windows/desktop/api/winsock2/nf-winsock2-closesocket
    virtual int closesocket(SOCKET s) const noexcept = 0;

    /// \see https://docs.microsoft.com/en-us/windows/desktop/api/synchapi/nf-synchapi-createeventa
    virtual HANDLE CreateEventA(LPSECURITY_ATTRIBUTES lpEventAttributes, BOOL bManualReset, BOOL bInitialState,
            LPCSTR lpName) const noexcept = 0;

    /// \see https://docs.microsoft.com/en-us/windows/desktop/api/fileapi/nf-fileapi-createfilea
    virtual HANDLE
    CreateFileA(LPCSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes,
            DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile
    ) const noexcept = 0;

    /// \see https://docs.microsoft.com/en-us/windows/desktop/api/winbase/nf-winbase-createnamedpipea
    virtual HANDLE
    CreateNamedPipeA(LPCSTR lpName, DWORD dwOpenMode, DWORD dwPipeMode, DWORD nMaxInstances, DWORD nOutBufferSize,
            DWORD nInBufferSize, DWORD nDefaultTimeOut, LPSECURITY_ATTRIBUTES lpSecurityAttributes) const noexcept = 0;

    /// \see https://docs.microsoft.com/en-us/windows/desktop/api/processthreadsapi/nf-processthreadsapi-createprocessw
    virtual BOOL
    CreateProcessA(LPCSTR lpApplicationName, LPSTR lpCommandLine, LPSECURITY_ATTRIBUTES lpProcessAttributes,
            LPSECURITY_ATTRIBUTES lpThreadAttributes, BOOL bInheritHandles, DWORD dwCreationFlags, LPVOID lpEnvironment,
            LPCSTR lpCurrentDirectory, LPSTARTUPINFOA lpStartupInfo,
            LPPROCESS_INFORMATION lpProcessInformation) const noexcept = 0;

    /// \see https://docs.microsoft.com/en-us/windows/desktop/api/ws2tcpip/nf-ws2tcpip-freeaddrinfo
    virtual VOID freeaddrinfo(PADDRINFOA pAddrInfo) const noexcept = 0;

    /// \see https://docs.microsoft.com/en-us/windows/desktop/api/Ws2tcpip/nf-ws2tcpip-getaddrinfo
    virtual INT
    getaddrinfo(PCSTR pNodeName, PCSTR pServiceName, const ADDRINFOA* pHints, PADDRINFOA* ppResult) const noexcept = 0;

    /// \see https://docs.microsoft.com/en-us/windows/desktop/api/errhandlingapi/nf-errhandlingapi-getlasterror
    virtual DWORD GetLastError() const noexcept = 0;

    /// \see https://docs.microsoft.com/en-us/windows/desktop/api/ioapiset/nf-ioapiset-getoverlappedresult
    virtual BOOL GetOverlappedResult(HANDLE hFile, LPOVERLAPPED lpOverlapped, LPDWORD lpNumberOfBytesTransferred,
            BOOL bWait) const noexcept = 0;

    /// \see https://docs.microsoft.com/en-us/windows/desktop/api/winsock2/nf-winsock2-listen
    virtual int listen(SOCKET s, int backlog) const noexcept = 0;

    /// \see https://docs.microsoft.com/en-us/windows/desktop/api/fileapi/nf-fileapi-readfile
    virtual BOOL ReadFile(HANDLE hFile, LPVOID lpBuffer, DWORD nNumberOfBytesToRead, LPDWORD lpNumberOfBytesRead,
            LPOVERLAPPED lpOverlapped) const noexcept = 0;

    /// \see https://docs.microsoft.com/en-us/windows/desktop/api/winsock2/nf-winsock2-recv
    virtual int recv(SOCKET s, char* buf, int len, int flags) const noexcept = 0;

    /// \see https://docs.microsoft.com/en-us/windows/desktop/api/synchapi/nf-synchapi-resetevent
    virtual BOOL ResetEvent(HANDLE hEvent) const noexcept = 0;

    /// \see https://docs.microsoft.com/en-us/windows/desktop/api/winsock2/nf-winsock2-send
    virtual int send(SOCKET s, const char* buf, int len, int flags) const noexcept = 0;

    /// \see https://docs.microsoft.com/en-us/windows/desktop/api/winsock2/nf-winsock2-select
    virtual int
    select(int nfds, fd_set* readfds, fd_set* writefds, fd_set* exceptfds, PTIMEVAL timeout) const noexcept = 0;

    /// \see https://docs.microsoft.com/en-us/windows/desktop/api/synchapi/nf-synchapi-setevent
    virtual BOOL SetEvent(HANDLE hEvent) const noexcept = 0;

    /// \see https://docs.microsoft.com/en-us/windows/desktop/api/winsock2/nf-winsock2-shutdown
    virtual int shutdown(SOCKET s, int how) const noexcept = 0;

    /// \see https://docs.microsoft.com/en-us/windows/desktop/api/winsock2/nf-winsock2-socket
    virtual SOCKET socket(int af, int type, int protocol) const noexcept = 0;

    /// \see https://docs.microsoft.com/en-us/windows/desktop/api/processthreadsapi/nf-processthreadsapi-terminateprocess
    virtual BOOL TerminateProcess(HANDLE hProcess, UINT uExitCode) const noexcept = 0;

    /// \see https://docs.microsoft.com/en-us/windows/desktop/api/synchapi/nf-synchapi-waitformultipleobjects
    virtual DWORD WaitForMultipleObjects(DWORD nCount, const HANDLE* lpHandles, BOOL bWaitAll,
            DWORD dwMilliseconds) const noexcept = 0;

    /// \see https://docs.microsoft.com/en-us/windows/desktop/api/synchapi/nf-synchapi-waitforsingleobject
    virtual DWORD WaitForSingleObject(HANDLE hHandle, DWORD dwMilliseconds) const noexcept = 0;

    /// \see https://docs.microsoft.com/en-us/windows/desktop/api/fileapi/nf-fileapi-writefile
    virtual BOOL WriteFile(HANDLE hFile, LPVOID lpBuffer, DWORD nNumberOfBytesToWrite, LPDWORD lpNumberOfBytesWritten,
            LPOVERLAPPED lpOverlapped) const noexcept = 0;

    /// \see https://docs.microsoft.com/en-us/windows/desktop/api/winsock2/nf-winsock2-wsacleanup
    virtual int WSACleanup() const noexcept = 0;

    /// \see https://docs.microsoft.com/en-us/windows/desktop/api/winsock2/nf-winsock2-wsagetlasterror
    virtual int WSAGetLastError() const noexcept = 0;

    /// \see https://docs.microsoft.com/en-us/windows/win32/api/winsock2/nf-winsock2-wsapoll
    virtual int WSAPoll(LPWSAPOLLFD fdArray, ULONG fds, INT timeout) const noexcept = 0;

    /// \see https://docs.microsoft.com/en-us/windows/desktop/api/winsock/nf-winsock-wsastartup
    virtual int WSAStartup(WORD wVersionRequired, LPWSADATA lpWSAData) const noexcept = 0;

    /// \see https://docs.microsoft.com/en-us/windows/console/generateconsolectrlevent
    virtual int GenerateConsoleCtrlEvent(DWORD pid) const noexcept = 0;
};

} // namespace wpwrapper

#endif // WPWRAPPER_API_H
