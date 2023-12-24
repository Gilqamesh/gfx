enum           hash_set_entry_type;
struct         hash_set_key;
typedef enum   hash_set_entry_type hash_set_entry_type_t;
typedef struct hash_set_key _hash_set_key_t;

enum hash_set_entry_type {
    HASH_SET_ENTRY_TYPE_EMPTY,
    HASH_SET_ENTRY_TYPE_TOMBSTONE,
    HASH_SET_ENTRY_TYPE_NON_EMPTY
};

// note: instead of storing entries in one node, separate entry from data
// [type, key][type, key][type, key] -> [type][type][type][key][key][key]
// this would decrease cache locality to find something in the hash table, but allow data to be contiguous

struct hash_set_key {
    enum hash_set_entry_type type;
    // ... key
};

static uint32_t _hash_set__entry_size(hash_set_t* self);
static _hash_set_key_t* hash_set__at(hash_set_t* self, uint32_t index);
static _hash_set_key_t* hash_set__key_to_internal_key(hash_set_key_t* key);
static hash_set_key_t* hash_set__internal_key_to_key(_hash_set_key_t* _key);
static _hash_set_key_t* _hash_set__find(hash_set_t* self, const hash_set_key_t* key);

static uint32_t _hash_set__entry_size(hash_set_t* self) {
    uint32_t result = sizeof(((_hash_set_key_t*)(0))->type) + self->size_of_key;
    return result;
}

static _hash_set_key_t* hash_set__at(hash_set_t* self, uint32_t index) {
    return (_hash_set_key_t*) ((char*) self->memory + _hash_set__entry_size(self) * index);
}

static _hash_set_key_t* hash_set__key_to_internal_key(hash_set_key_t* key) {
    return (_hash_set_key_t*) ((char*) key - sizeof(((_hash_set_key_t*)(0))->type));
}

static hash_set_key_t* hash_set__internal_key_to_key(_hash_set_key_t* _key) {
    return (hash_set_key_t*) ((char*) _key + sizeof(((_hash_set_key_t*)(0))->type));
}

static _hash_set_key_t* _hash_set__find(hash_set_t* self, const hash_set_key_t* key) {
    const uint32_t capacity = hash_set__capacity(self);
    uint32_t index = self->hash_fn(key) % capacity;
    const uint32_t start_index = index;
    _hash_set_key_t* tombstone = NULL;
    while (true) {
        _hash_set_key_t* _found_key = hash_set__at(self, index);
        switch (_found_key->type) {
            case HASH_SET_ENTRY_TYPE_EMPTY: {
                return tombstone ? tombstone : _found_key;
            } break ;
            case HASH_SET_ENTRY_TYPE_TOMBSTONE: {
                if (!tombstone) {
                    tombstone = _found_key;
                }
            } break ;
            case HASH_SET_ENTRY_TYPE_NON_EMPTY: {
                if (self->eq_fn(key, hash_set__internal_key_to_key(_found_key))) {
                    return _found_key;
                }
            } break ;
            default: assert(false);
        }
        
        index = (index + 1) % capacity;
        if (index == start_index) {
            // full
            return NULL;
        }
    }

    assert(false);
}
