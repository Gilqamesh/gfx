#ifndef PACKET_H
# define PACKET_H

struct         packet;
struct         game_data;
typedef struct packet    packet_t;
typedef struct game_data game_data_t;

struct game_data {
    uint32_t buttons;
};

struct packet {
    uint32_t    sequence_id;
    game_data_t game_data;
};

#endif // PACKET_H
