#ifndef HASH_SET_H
# define HASH_SET_H

# include <stdint.h>
# include <stdbool.h>

typedef void hash_set_key_t;

// static hash_set
typedef struct hash_set {
    uint32_t            size_of_key;
    uint32_t            fill;
    uint32_t            (*hash_fn)(const hash_set_key_t*);
    bool                (*eq_fn)(const hash_set_key_t*, const hash_set_key_t*);
    void*               memory;
    uint64_t            memory_size;
} hash_set_t;

// use this to measure how much memory is necessary
uint32_t hash_set__entry_size(uint32_t size_of_key);
uint32_t hash_set__hash_fn_string(const hash_set_key_t* string_key);
bool hash_set__eq_fn_string(const hash_set_key_t* string_key_a, const hash_set_key_t* string_key_b);

bool hash_set__create(
    hash_set_t* self,
    void* memory,
    uint64_t memory_size,
    uint32_t size_of_key,
    uint32_t (*hash_fn)(const hash_set_key_t*),
    bool (*eq_fn)(const hash_set_key_t*, const hash_set_key_t*)
);

hash_set_key_t* hash_set__insert(hash_set_t* self, const hash_set_key_t* key);
bool hash_set__remove(hash_set_t* self, const hash_set_key_t* key);
void hash_set__clear(hash_set_t* self);
hash_set_key_t* hash_set__find(hash_set_t* self, const hash_set_key_t* key);

uint32_t hash_set__size(hash_set_t* self);
uint32_t hash_set__capacity(hash_set_t* self);

hash_set_key_t* hash_set__begin(hash_set_t* self);
hash_set_key_t* hash_set__next(hash_set_t* self, hash_set_key_t* key);
hash_set_key_t* hash_set__end(hash_set_t* self);

#endif // HASH_SET_H
