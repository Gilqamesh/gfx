#include "network.h"

#include <stdio.h>

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <ifaddrs.h>

struct network_id {
    int32_t socket;
};

network_id_t network_id__create_server(uint16_t port) {
    int32_t socket_fd = socket(AF_INET, SOCK_DGRAM | SOCK_NONBLOCK, IPPROTO_UDP);
    if (socket_fd == -1) {
        perror(0);
        return 0;
    }

    // struct ifaddrs* network_interfaces = 0;
    // if (getifaddrs(&network_interfaces) == -1) {
    //     return 0;
    // }
    // struct ifaddrs* cur_network_interface = network_interfaces;
    // printf("Network interfaces:\n");
    // while (cur_network_interface) {
    //     printf("  name: %s\n", cur_network_interface->ifa_name);
    //     struct sockaddr* sock_addr = cur_network_interface->ifa_addr;
    //         char addr[128] = { 0 };
    //         if (inet_ntop(sock_addr->sa_family, (const void*) sock_addr, addr, sizeof(addr))) {
    //             printf("  addr: %s\n", addr);
    //         }
    //     if (sock_addr->sa_family == AF_INET) {
    //         struct sockaddr_in* sock_addr_in = (struct sockaddr_in*) sock_addr;
    //         sock_addr_in->sin_addr;
    //     }
    //     cur_network_interface = cur_network_interface->ifa_next;
    // }
    // freeifaddrs(network_interfaces);

    struct sockaddr_in src_addr;
    src_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    src_addr.sin_family = AF_INET;
    src_addr.sin_port = htons(port);

    if (bind(socket_fd, (const struct sockaddr*) &src_addr, sizeof(src_addr)) == -1) {
        perror(0);
        return 0;
    }

    network_id_t result = calloc(1, sizeof(*result));
    if (!result) {
        perror(0);
        return 0;
    }

    result->socket = socket_fd;

    return result;
}

network_id_t network_id__create() {
    int32_t socket_fd = socket(AF_INET, SOCK_DGRAM | SOCK_NONBLOCK, IPPROTO_UDP);
    if (socket_fd == -1) {
        perror(0);
        return 0;
    }

    network_id_t result = calloc(1, sizeof(*result));
    if (!result) {
        perror(0);
        return 0;
    }

    result->socket = socket_fd;

    return result;
}

void network_id__destroy(network_id_t self) {
    close(self->socket);
}

bool network__connect(network_id_t self, const char* ip, uint16_t port) {
    in_addr_t dst_in_addr = inet_addr(ip);
    if (dst_in_addr == INADDR_NONE) {
        return false;
    }

    struct sockaddr_in dst_addr = {
        .sin_addr.s_addr = dst_in_addr,
        .sin_family = AF_INET,
        .sin_port = htons(port)
    };
    if (connect(self->socket, (const struct sockaddr*) &dst_addr, sizeof(dst_addr)) == -1) {
        perror(0);
        return false;
    }

    return true;
}

bool network_id__send_data(network_id_t self, const void* data, uint32_t data_size) {
    if (send(self->socket, data, data_size, MSG_DONTWAIT) == -1) {
        return false;
    }

    return true;
}

bool network_id__get_data(network_id_t self, void* out_buffer, uint32_t out_buffer_size) {
    struct sockaddr src_addr;
    socklen_t src_addr_len_original = sizeof(src_addr);
    socklen_t src_addr_len = src_addr_len_original;
    ssize_t messsage_len = recvfrom(self->socket, out_buffer, out_buffer_size, MSG_DONTWAIT, &src_addr, &src_addr_len);
    if (messsage_len == -1) {
        return false;
    }

    assert(messsage_len < out_buffer_size);

    if (src_addr_len == src_addr_len_original) {
        // determine src_addr
    }

    return true;
}
