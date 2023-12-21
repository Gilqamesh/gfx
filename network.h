#ifndef NETWORK_H
# define NETWORK_H

# include <stdint.h>
# include <stdbool.h>

struct         network_id;
typedef struct network_id* network_id_t;

//! @param ip if NULL, listens on any incoming connection
network_id_t network_id__create_server(uint16_t port);
network_id_t network_id__create();
void network_id__destroy(network_id_t self);

bool network_id__send_data(network_id_t self, const char* ip, uint16_t port, const void* data, uint32_t data_size);
bool network_id__get_data(network_id_t self, void* out_buffer, uint32_t out_buffer_size);

#endif // NETWORK_H
