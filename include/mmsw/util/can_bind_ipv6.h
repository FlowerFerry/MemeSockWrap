
#ifndef MMSW_UTIL_CAN_IPV6_BIND_H_INCLUDED
#define MMSW_UTIL_CAN_IPV6_BIND_H_INCLUDED

#include <mego/predef/symbol/inline.h>
#include <mego/predef/os/windows.h>

#include <stdbool.h>
#include <stdint.h>

#if MG_OS__WIN_AVAIL
#  include <winsock2.h>
#  include <ws2tcpip.h>
#  pragma comment(lib, "ws2_32.lib")
#else
#  include <sys/socket.h>
#  include <netinet/in.h>
#  include <unistd.h>
#endif

MG_CAPI_INLINE bool mmsw_util__can_bind_ipv6(uint16_t _port)
{
#if MG_OS__WIN_AVAIL
    WSADATA wsaData;
    SOCKET sock = INVALID_SOCKET;
    int ret = SOCKET_ERROR;
    struct sockaddr_in6 addr;
    int opt = 1;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) 
    {
        return false;
    }

    sock = socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP);
    if (sock == INVALID_SOCKET) {
        WSACleanup();
        return false;
    }

    // option SO_REUSEADDR
    ret = setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt));
    if (ret == SOCKET_ERROR) {
        closesocket(sock);
        WSACleanup();
        return false;
    }

    memset(&addr, 0, sizeof(addr));
    addr.sin6_family = AF_INET6;
    addr.sin6_port = htons(_port);
    addr.sin6_addr = in6addr_any;

    ret = bind(sock, (struct sockaddr*)&addr, sizeof(addr));
    closesocket(sock);
    WSACleanup();
    return ret != SOCKET_ERROR;
#else // posix
    int sock = -1;
    int ret = -1;
    struct sockaddr_in6 addr;
    int opt = 1;
    sock = socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP);
    if (sock == -1) {
        return false;
    }

    // option SO_REUSEADDR
    ret = setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    if (ret == -1) {
        close(sock);
        return false;
    }

    memset(&addr, 0, sizeof(addr));
    addr.sin6_family = AF_INET6;
    addr.sin6_port = htons(_port);
    addr.sin6_addr = in6addr_any;

    ret = bind(sock, (struct sockaddr*)&addr, sizeof(addr));
    close(sock);
    return ret != -1;
#endif
}

#endif // !MMSW_UTIL_CAN_IPV6_BIND_H_INCLUDED
