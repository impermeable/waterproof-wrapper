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

SOCKET api_wrapper::accept(SOCKET s, sockaddr* addr, int* addrlen) const noexcept
{
    return ::accept(s, addr, addrlen);
}

int api_wrapper::bind(SOCKET s, const sockaddr* name, int namelen) const noexcept
{
    return ::bind(s, name, namelen);
}

BOOL api_wrapper::CancelIoEx(HANDLE hFile, LPOVERLAPPED lpOverlapped) const noexcept
{
    return ::CancelIoEx(hFile, lpOverlapped);
}

BOOL api_wrapper::CloseHandle(HANDLE hObject) const noexcept
{
    return ::CloseHandle(hObject);
}

int api_wrapper::closesocket(SOCKET s) const noexcept
{
    return ::closesocket(s);
}

HANDLE api_wrapper::CreateEventA(LPSECURITY_ATTRIBUTES lpEventAttributes, BOOL bManualReset, BOOL bInitialState,
        LPCSTR lpName) const noexcept
{
    return ::CreateEventA(lpEventAttributes, bManualReset, bInitialState, lpName);
}

HANDLE api_wrapper::CreateFileA(LPCSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode,
        LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes,
        HANDLE hTemplateFile) const noexcept
{
    return ::CreateFileA(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition,
            dwFlagsAndAttributes, hTemplateFile);
}

HANDLE api_wrapper::CreateNamedPipeA(LPCSTR lpName, DWORD dwOpenMode, DWORD dwPipeMode, DWORD nMaxInstances,
        DWORD nOutBufferSize, DWORD nInBufferSize, DWORD nDefaultTimeOut,
        LPSECURITY_ATTRIBUTES lpSecurityAttributes) const noexcept
{
    return ::CreateNamedPipeA(lpName, dwOpenMode, dwPipeMode, nMaxInstances, nOutBufferSize, nInBufferSize,
            nDefaultTimeOut, lpSecurityAttributes);
}

BOOL api_wrapper::CreateProcessA(LPCSTR lpApplicationName, LPSTR lpCommandLine,
        LPSECURITY_ATTRIBUTES lpProcessAttributes, LPSECURITY_ATTRIBUTES lpThreadAttributes, BOOL bInheritHandles,
        DWORD dwCreationFlags, LPVOID lpEnvironment, LPCSTR lpCurrentDirectory, LPSTARTUPINFOA lpStartupInfo,
        LPPROCESS_INFORMATION lpProcessInformation) const noexcept
{
    return ::CreateProcessA(lpApplicationName, lpCommandLine, lpProcessAttributes, lpThreadAttributes, bInheritHandles,
            dwCreationFlags, lpEnvironment, lpCurrentDirectory, lpStartupInfo, lpProcessInformation);
}

VOID api_wrapper::freeaddrinfo(PADDRINFOA pAddrInfo) const noexcept
{
    return ::freeaddrinfo(pAddrInfo);
}

INT api_wrapper::getaddrinfo(PCSTR pNodeName, PCSTR pServiceName, const ADDRINFOA* pHints,
        PADDRINFOA* ppResult) const noexcept
{
    return ::getaddrinfo(pNodeName, pServiceName, pHints, ppResult);
}

DWORD api_wrapper::GetLastError() const noexcept
{
    return ::GetLastError();
}

BOOL api_wrapper::GetOverlappedResult(HANDLE hFile, LPOVERLAPPED lpOverlapped, LPDWORD lpNumberOfBytesTransferred,
        BOOL bWait) const noexcept
{
    return ::GetOverlappedResult(hFile, lpOverlapped, lpNumberOfBytesTransferred, bWait);
}

int api_wrapper::listen(SOCKET s, int backlog) const noexcept
{
    return ::listen(s, backlog);
}

BOOL api_wrapper::ReadFile(HANDLE hFile, LPVOID lpBuffer, DWORD nNumberOfBytesToRead, LPDWORD lpNumberOfBytesRead,
        LPOVERLAPPED lpOverlapped) const noexcept
{
    return ::ReadFile(hFile, lpBuffer, nNumberOfBytesToRead,
            lpNumberOfBytesRead, lpOverlapped);
}

int api_wrapper::recv(SOCKET s, char* buf, int len, int flags) const noexcept
{
    return ::recv(s, buf, len, flags);
}

BOOL api_wrapper::ResetEvent(HANDLE hEvent) const noexcept
{
    return ::ResetEvent(hEvent);
}

int api_wrapper::send(SOCKET s, const char* buf, int len, int flags) const noexcept
{
    return ::send(s, buf, len, flags);
}

int api_wrapper::select(int nfds, fd_set* readfds, fd_set* writefds, fd_set* exceptfds, PTIMEVAL timeout) const noexcept
{
    return ::select(nfds, readfds, writefds, exceptfds, timeout);
}

BOOL api_wrapper::SetEvent(HANDLE hEvent) const noexcept
{
    return ::SetEvent(hEvent);
}

int api_wrapper::shutdown(SOCKET s, int how) const noexcept
{
    return ::shutdown(s, how);
}

SOCKET api_wrapper::socket(int af, int type, int protocol) const noexcept
{
    return ::socket(af, type, protocol);
}

BOOL api_wrapper::TerminateProcess(HANDLE hProcess, UINT uExitCode) const noexcept
{
    return ::TerminateProcess(hProcess, uExitCode);
}

DWORD api_wrapper::WaitForMultipleObjects(DWORD nCount, const HANDLE* lpHandles, BOOL bWaitAll,
        DWORD dwMilliseconds) const noexcept
{
    return ::WaitForMultipleObjects(nCount, lpHandles, bWaitAll, dwMilliseconds);
}

DWORD api_wrapper::WaitForSingleObject(HANDLE hHandle, DWORD dwMilliseconds) const noexcept
{
    return ::WaitForSingleObject(hHandle, dwMilliseconds);
}

BOOL
api_wrapper::WriteFile(HANDLE hFile, LPVOID lpBuffer, DWORD nNumberOfBytesToWrite, LPDWORD lpNumberOfBytesWritten,
        LPOVERLAPPED lpOverlapped) const noexcept
{
    return ::WriteFile(hFile, lpBuffer, nNumberOfBytesToWrite,
            lpNumberOfBytesWritten, lpOverlapped);
}

int api_wrapper::WSACleanup() const noexcept
{
    return ::WSACleanup();
}

int api_wrapper::WSAGetLastError() const noexcept
{
    return ::WSAGetLastError();
}

int api_wrapper::WSAPoll(LPWSAPOLLFD fdArray, ULONG fds, INT timeout) const noexcept
{
    return ::WSAPoll(fdArray, fds, timeout);
}

int api_wrapper::WSAStartup(WORD wVersionRequired, LPWSADATA lpWSAData) const noexcept
{
    return ::WSAStartup(wVersionRequired, lpWSAData);
}

int api_wrapper::GenerateConsoleCtrlEvent(DWORD pid) const noexcept
{
  return ::GenerateConsoleCtrlEvent(CTRL_BREAK_EVENT, pid);
}

} // namespace wpwrapper
