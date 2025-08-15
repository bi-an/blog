getaddrinfo("localhost", "sunrpc", &hints, &result); // "sunrpc" 或 端口 111 都可以
for (const auto *rp = result; rp != nullptr; rp = rp->ai_next) {
    sock = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
    if (sock == -1) continue;
    if (connect(sock, rp->ai_addr, rp->ai_addrlen) != -1) break;  // Success
    perror("connect failed");
    close(sock);
    sock = -1;
}