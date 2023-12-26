#ifndef TP_H
# define TP_H

# include <stdint.h>
# include <stdbool.h>

struct         tp_socket;
struct         network_addr;
enum           socket_type;
typedef struct tp_socket    tp_socket_t;
typedef struct network_addr network_addr_t;
typedef enum   socket_type  socket_type_t;

struct tp_socket {
    int32_t socket;
};

struct network_addr {
    uint32_t addr;
    uint32_t port;
};

enum socket_type {
    SOCKET_TYPE_UDP,
    SOCKET_TYPE_TCP
};

bool network_addr__create(network_addr_t* self, const char* ip, uint16_t port);
bool network_addr__is_same(network_addr_t* self, network_addr_t* other);

bool tp_socket__create(tp_socket_t* self, socket_type_t type, uint16_t port);

// bool tp_socket__create(tp_socket_t* self);
void tp_socket__destroy(tp_socket_t* self);

/**
 * @brief Even though there isn't a connection in UDP, this can be used along with tp_socket__send_data to avoid passing in the destination
*/
bool tp_socket__connect(tp_socket_t* self, network_addr_t addr);
bool tp_socket__send_data(tp_socket_t* self, const void* data, uint32_t data_size);

bool tp_socket__send_data_to(tp_socket_t* self, const void* data, uint32_t data_size, network_addr_t dst_info);
bool tp_socket__get_data(tp_socket_t* self, void* data, uint32_t data_size, uint32_t* data_len, network_addr_t* sender_addr);

#endif // TP_H
