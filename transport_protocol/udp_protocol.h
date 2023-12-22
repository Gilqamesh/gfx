#ifndef UDP_PROTOCOL_H
# define UDP_PROTOCOL_H

# include <stdint.h>
# include <stdbool.h>

struct         network_id;
struct         network_info;
typedef struct network_id* network_id_t;
typedef struct network_info network_info_t;

struct network_info {
    uint32_t addr;
    uint32_t port;
};

//! @param ip if NULL, listens on any incoming connection
network_id_t network_id__create_server(uint16_t port);
network_id_t network_id__create();
void network_id__destroy(network_id_t self);

bool network__connect(network_id_t self, const char* ip, uint16_t port);
bool network_id__send_data(network_id_t self, const void* data, uint32_t data_size);

bool network_id__send_data_to(network_id_t self, const void* data, uint32_t data_size, const network_info_t* dst_info);

bool network_id__get_data(network_id_t self, void* out_data, uint32_t out_data_size, uint32_t* out_data_len, network_info_t* out_sender_info);

#endif // UDP_PROTOCOL_H
