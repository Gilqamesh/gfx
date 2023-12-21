#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "network.h"
#include "system.h"

int main() {
    system__init();

    network_id_t network_id = network_id__create();
    if (!network_id) {
        exit(1);
    }

    printf("Created network id\n");

    const char* server_ip = "127.0.0.1";
    uint16_t    server_port = 3200;
    if (!network__connect(network_id, server_ip, server_port)) {
        network_id__destroy(network_id);
        exit(2);
    }

    printf("Connected to %s:%u\n", server_ip, server_port);

    const char* msg = "Hello from client";
    uint32_t msg_len = strlen(msg);

    uint32_t messages_sent = 0;
    while (true) {
        if (network_id__send_data(network_id, msg, msg_len)) {
            printf("#%3d Sent message to server\n", messages_sent);
            ++messages_sent;
        }
        system__sleep(1.0);
    }

    network_id__destroy(network_id);

    return 0;
}
