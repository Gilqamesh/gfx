#include "udp.h"

#include <stdio.h>

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <ifaddrs.h>
#include <errno.h>

bool network_addr__create(network_addr_t* self, const char* ip, uint16_t port) {
    in_addr_t dst_in_addr = inet_addr(ip);
    if (dst_in_addr == INADDR_NONE) {
        return false;
    }

    self->addr = dst_in_addr;
    self->port = htons(port);

    return true;
}

bool udp_socket__create(udp_socket_t* self, uint16_t port) {
    struct sockaddr_in src_addr;
    src_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    src_addr.sin_family = AF_INET;
    src_addr.sin_port = htons(port);

    int32_t socket_fd = socket(AF_INET, SOCK_DGRAM | SOCK_NONBLOCK, IPPROTO_UDP);
    if (socket_fd == -1) {
        perror(0);
        return false;
    }
    int opt = 1;
    if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, (const void*) &opt, sizeof(opt)) != 0) {
        perror(0);
        close(socket_fd);
        return false;
    }

    if (bind(socket_fd, (const struct sockaddr*) &src_addr, sizeof(src_addr)) == -1) {
        fprintf(stderr, "%d\n", errno);
        perror(0);
        return false;
    }

    self->socket = socket_fd;

    return true;
}

void udp_socket__destroy(udp_socket_t* self) {
    close(self->socket);
}

bool udp_socket__connect(udp_socket_t* self, network_addr_t addr) {
    struct sockaddr_in dst_addr = { 0 };
    dst_addr.sin_addr.s_addr = addr.addr;
    dst_addr.sin_family = AF_INET;
    dst_addr.sin_port = addr.port;

    if (connect(self->socket, (const struct sockaddr*) &dst_addr, sizeof(dst_addr)) == -1) {
        perror(0);
        return false;
    }

    return true;
}

bool udp_socket__send_data(udp_socket_t* self, const void* data, uint32_t data_size) {
    if (send(self->socket, data, data_size, MSG_DONTWAIT) == -1) {
        return false;
    }

    return true;
}

bool udp_socket__send_data_to(udp_socket_t* self, const void* data, uint32_t data_size, network_addr_t dst_info) {
    struct sockaddr_in dst_addr = { 0 };
    dst_addr.sin_addr.s_addr = dst_info.addr;
    dst_addr.sin_family = AF_INET;
    dst_addr.sin_port = dst_info.port;
    if (sendto(self->socket, data, data_size, MSG_DONTWAIT, (const struct sockaddr*) &dst_addr, sizeof(dst_addr)) == -1) {
        return false;
    }

    return true;
}

bool udp_socket__get_data(udp_socket_t* self, void* data, uint32_t data_size, uint32_t* data_len, network_addr_t* sender_addr) {
    struct sockaddr src_addr;
    socklen_t src_addr_len_original = sizeof(src_addr);
    socklen_t src_addr_len = src_addr_len_original;
    ssize_t message_len = recvfrom(self->socket, data, data_size, MSG_DONTWAIT, &src_addr, &src_addr_len);
    if (message_len == -1) {
        return false;
    }

    if (data_len) {
        *data_len = message_len;
    }

    if (src_addr.sa_family == AF_INET && src_addr_len == src_addr_len_original && sender_addr) {
        struct sockaddr_in* src_addr_in = (struct sockaddr_in*) &src_addr;
        sender_addr->addr = src_addr_in->sin_addr.s_addr;
        sender_addr->port = src_addr_in->sin_port;
    }

    return true;
}
