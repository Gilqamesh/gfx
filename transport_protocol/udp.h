#ifndef UDP_H
# define UDP_H

# include <stdint.h>
# include <stdbool.h>

struct         udp_socket;
struct         network_addr;
typedef struct udp_socket   udp_socket_t;
typedef struct network_addr network_addr_t;

struct udp_socket {
    int32_t socket;
};

struct network_addr {
    uint32_t addr;
    uint32_t port;
};

bool udp_socket__create(udp_socket_t* self, uint16_t port);

// bool udp_socket__create(udp_socket_t* self);
void udp_socket__destroy(udp_socket_t* self);

/**
 * @brief Even though there isn't a connection in UDP, this can be used along with udp_socket__send_data to avoid passing in the destination
*/
bool udp_socket__connect(udp_socket_t* self, const char* ip, uint16_t port);
bool udp_socket__send_data(udp_socket_t* self, const void* data, uint32_t data_size);

bool udp_socket__send_data_to(udp_socket_t* self, const void* data, uint32_t data_size, const network_addr_t* dst_info);
bool udp_socket__get_data(udp_socket_t* self, void* out_data, uint32_t out_data_size, uint32_t* out_data_len, network_addr_t* out_sender_info);

#endif // UDP_H
