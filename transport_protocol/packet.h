#ifndef PACKET_H
# define PACKET_H

# include "tp.h"

# include <stdbool.h>

struct         packet;
struct         game_data;
struct         connection;
typedef struct packet     packet_t;
typedef struct game_data  game_data_t;
typedef struct connection connection_t;
typedef uint16_t          seq_id_t;

struct game_data {
    uint32_t buttons;
};

#define PACKET_FORMAT "[seq: %-5u ack: %-5u]"
struct packet {
    seq_id_t    sequence_id;
    seq_id_t    ack;
    /**
     * a bit is 1 if that sequence id was acknowledged, 0 otherwise
     * bits represent: [ack - 1, ack - 2, ..., ack - 31]
    */
    uint32_t    ack_bitfield;
    game_data_t game_data;
};

struct connection {
    network_addr_t addr;
    double         time_last_sent;
    seq_id_t       sequence_id;
    uint32_t       ack_bitfield_queue;
    bool           connected;
};

static inline bool sequence_id__is_more_recent(seq_id_t seq_id_1, seq_id_t seq_id_2) {
    return (
        (seq_id_1 > seq_id_2 && seq_id_1 - seq_id_2 <= (((seq_id_t)-1) >> 1)) ||
        (seq_id_2 > seq_id_1 && seq_id_2 - seq_id_1 >  (((seq_id_t)-1) >> 1))
    );
}

/**
 * @returns Number of packages between 'newer_seq_id' and 'older_seq_id'
*/
static inline seq_id_t sequence_id__delta(seq_id_t newer_seq_id, seq_id_t older_seq_id) {
    return newer_seq_id - older_seq_id;
}

#endif // PACKET_H
