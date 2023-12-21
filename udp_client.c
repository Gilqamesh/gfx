#include <stdio.h>
#include <string.h>

#include "network.h"
#include "system.h"

int main() {
    system__init();

    network_id_t network_id = network_id__create();

    const char* msg = "Hello from client";
    uint32_t msg_len = strlen(msg);

    const char* server_ip = "127.0.0.1";
    uint16_t    server_port = 3200;
    printf("Sending data to %s:%u\n", server_ip, server_port);
    uint32_t loop_counter = 0;
    while (true) {
        if (network_id__send_data(network_id, server_ip, server_port, msg, msg_len)) {
            printf("#%3d Sent message to server\n", loop_counter);
        }
        ++loop_counter;
        system__sleep(1.0);
    }

    network_id__destroy(network_id);

    return 0;
}
