#ifndef HASH_MAP_H
# define HASH_MAP_H

# include <stdint.h>
# include <stdbool.h>

typedef void hash_map_key_t;
typedef void hash_map_value_t;

// static hash_map
typedef struct hash_map {
    uint32_t            size_of_key;
    uint32_t            size_of_value;
    uint32_t            fill;
    uint32_t            (*hash_fn)(const hash_map_key_t*);
    bool                (*eq_fn)(const hash_map_key_t*, const hash_map_key_t*);
    void*               memory;
    uint64_t            memory_size;
} hash_map_t;

// use this to measure how much memory is necessary
uint32_t hash_map__entry_size(uint32_t size_of_key, uint32_t size_of_value);
uint32_t hash_fn__string(const hash_map_key_t* string_key);
bool eq_fn__string(const hash_map_key_t* string_key_a, const hash_map_key_t* string_key_b);

bool hash_map__create(
    hash_map_t* self,
    void* memory, uint64_t memory_size,
    uint32_t size_of_key, uint32_t size_of_value,
    uint32_t (*hash_fn)(const hash_map_key_t*),
    bool (*eq_fn)(const hash_map_key_t*, const hash_map_key_t*)
);

hash_map_key_t* hash_map__insert(hash_map_t* self, const hash_map_key_t* key, const hash_map_value_t* value);
bool hash_map__remove(hash_map_t* self, const hash_map_key_t* key);
void hash_map__clear(hash_map_t* self);
hash_map_value_t* hash_map__find(hash_map_t* self, const hash_map_key_t* key);

uint32_t hash_map__size(hash_map_t* self);
uint32_t hash_map__capacity(hash_map_t* self);

hash_map_key_t* hash_map__begin(hash_map_t* self);
hash_map_key_t* hash_map__next(hash_map_t* self, hash_map_key_t* key);
hash_map_key_t* hash_map__end(hash_map_t* self);

hash_map_value_t* hash_map__value(hash_map_t* self, hash_map_key_t* key);
hash_map_key_t* hash_map__key(hash_map_t* self, hash_map_value_t* value);

#endif // HASH_MAP_H
