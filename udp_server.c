#include <stdio.h>
#include <stdlib.h>

#include "network.h"

int main() {
    network_id_t network_id = network_id__create_server(3200);
    if (!network_id) {
        exit(1);
    }

    char buffer[128] = { 0 };

    printf("Listening to incoming data..\n");
    while (true) {
        if (network_id__get_data(network_id, (void*) buffer, sizeof(buffer) - 1)) {
            printf("Received data: %s\n");
        }
    }

    network_id__destroy(network_id);

    return 0;
}
