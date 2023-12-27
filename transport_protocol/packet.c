#include "packet.h"

#include "debug.h"

void debug__write_ack_bitfield_raw(uint32_t ack_bitfield) {
    for (int32_t bit_index = (sizeof(ack_bitfield) << 3) - 1; bit_index >= 0; --bit_index) {
        uint32_t bit = (ack_bitfield & (1 << bit_index)) >> bit_index;
        debug__write_raw("%c", bit ? '1' : '0');
        if (bit_index > 0 && bit_index % 8 == 0) {
            debug__write_raw(" ");
        }
    }
}

void debug__write_packet_raw(packet_t* packet) {
    debug__write_raw("%-10u%-10u", packet->sequence_id, packet->ack);
    debug__write_ack_bitfield_raw(packet->ack_bitfield);
    debug__write_raw("\n");
}

bool sequence_id__is_more_recent(seq_id_t seq_id_1, seq_id_t seq_id_2) {
    return (
        (seq_id_1 > seq_id_2 && seq_id_1 - seq_id_2 <= (((seq_id_t)-1) >> 1)) ||
        (seq_id_2 > seq_id_1 && seq_id_2 - seq_id_1 >  (((seq_id_t)-1) >> 1))
    );
}

seq_id_t sequence_id__delta(seq_id_t newer_seq_id, seq_id_t older_seq_id) {
    return newer_seq_id - older_seq_id;
}

seq_id_t sequence_id__sub(seq_id_t seq_id_1, uint32_t val) {
    return (seq_id_t) (seq_id_1 - val);
}
