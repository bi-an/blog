---
title: Network discovery
date: 2024-01-09 16:15:17
categories: Network
tags: tcp/ip
---

## Get IP from the host name

Key function: `getaddrinfo`.

```cpp
#define _POSIX_C_SOURCE 200112L

#include <stdio.h>
#include <sys/socket.h>
#include <netdb.h>
#include <sys/types.h>
#include <errno.h>
#include <strings.h>
#include <arpa/inet.h>

int main() {
    struct addrinfo hints;
    bzero(&hints, sizeof hints);
    // hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    struct addrinfo *res = 0;
    // The following 3 hostnames are legal.
    int rc = getaddrinfo("www.baidu.com", NULL, &hints, &res);
    // int rc = getaddrinfo("localhost", NULL, &hints, &res);
    // int rc = getaddrinfo("PC-XXX", NULL, &hints, &res); // PC-XXX is a hostname
    if (rc != 0)
        perror("getaddrinfo failed");
    
    for (struct addrinfo* res_i = res; res_i != NULL; res_i = res_i->ai_next) {
        if (res_i->ai_addr) {
            if (res_i->ai_addr->sa_family == AF_INET) {
                char ip4[INET_ADDRSTRLEN];
                inet_ntop(AF_INET, &(((struct sockaddr_in*)(res_i->ai_addr))->sin_addr), ip4, INET_ADDRSTRLEN);
                printf("IP: %s\n", ip4);
            } else {
                char ip6[INET6_ADDRSTRLEN];
                inet_ntop(AF_INET6, &(((struct sockaddr_in6*)(res_i->ai_addr))->sin6_addr), ip6, INET_ADDRSTRLEN);
                printf("IP: %s\n", ip6);
            }
        }
    }

    return 0;
}
```
