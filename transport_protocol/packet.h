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
    double         time_last_seen;
    seq_id_t       sequence_id;
    uint32_t       ack_bitfield;
    uint32_t       packets_dropped;
    // uint32_t       packets_received; // todo: this, and throughput
    double         rtt;
    double         time_connected;
    bool           connected;
};

void debug__write_ack_bitfield_raw(uint32_t ack_bitfield);
void debug__write_packet_raw(packet_t* packet);

bool sequence_id__is_more_recent(seq_id_t seq_id_1, seq_id_t seq_id_2);

/**
 * @returns Number of packages between 'newer_seq_id' and 'older_seq_id'
*/
seq_id_t sequence_id__delta(seq_id_t newer_seq_id, seq_id_t older_seq_id);

seq_id_t sequence_id__sub(seq_id_t seq_id_1, uint32_t val);

#endif // PACKET_H
