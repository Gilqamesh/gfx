// note: instead of storing entries in one node, separate entry from data
// [type, key, value][type, key, value][type, key, value] -> [type][type][type][key][key][key][value][value][value]
// this would decrease cache locality to find something in the hash table, but allow data to be contiguous

enum           hash_map_entry_type;
struct         _hash_map_entry;
typedef enum   hash_map_entry_type hash_map_entry_type_t;
typedef struct _hash_map_entry     _hash_map_entry_t;

enum hash_map_entry_type {
    HASH_MAP_ENTRY_TYPE_EMPTY,
    HASH_MAP_ENTRY_TYPE_TOMBSTONE,
    HASH_MAP_ENTRY_TYPE_NON_EMPTY
};

struct _hash_map_entry {
    enum hash_map_entry_type type;
    // ... key
    // ... value
};

static uint32_t _hash_map__entry_size(hash_map_t* self);
static _hash_map_entry_t* hash_map__at(hash_map_t* self, uint32_t index);
static _hash_map_entry_t* hash_map__key_to_internal_entry(hash_map_key_t* key);
static _hash_map_entry_t* hash_map__value_to_internal_entry(hash_map_t* self, hash_map_value_t* value);
static hash_map_key_t* hash_map__internal_entry_to_key(_hash_map_entry_t* _entry);
static hash_map_value_t* hash_map__internal_entry_to_value(hash_map_t* self, _hash_map_entry_t* _entry);
static _hash_map_entry_t* _hash_map__find(hash_map_t* self, const hash_map_key_t* key);

static uint32_t _hash_map__entry_size(hash_map_t* self) {
    uint32_t result = sizeof(((_hash_map_entry_t*)(0))->type) + self->size_of_key + self->size_of_value;
    return result;
}

static _hash_map_entry_t* hash_map__at(hash_map_t* self, uint32_t index) {
    return (_hash_map_entry_t*) ((char*) self->memory + _hash_map__entry_size(self) * index);
}

static _hash_map_entry_t* hash_map__key_to_internal_entry(hash_map_key_t* key) {
    return (_hash_map_entry_t*) ((char*) key - sizeof(((_hash_map_entry_t*)(0))->type));
}

static _hash_map_entry_t* hash_map__value_to_internal_entry(hash_map_t* self, hash_map_value_t* value) {
    return (_hash_map_entry_t*) ((char*) value - self->size_of_key - sizeof(((_hash_map_entry_t*)(0))->type));
}

static hash_map_key_t* hash_map__internal_entry_to_key(_hash_map_entry_t* _entry) {
    return (hash_map_key_t*)((char*) _entry + sizeof(((_hash_map_entry_t*)(0))->type));
}

static hash_map_value_t* hash_map__internal_entry_to_value(hash_map_t* self, _hash_map_entry_t* _entry) {
    return (hash_map_value_t*)((char*) _entry + sizeof(((_hash_map_entry_t*)(0))->type) + self->size_of_key);
}

static _hash_map_entry_t* _hash_map__find(hash_map_t* self, const hash_map_key_t* key) {
    const uint32_t capacity = hash_map__capacity(self);
    uint32_t index = self->hash_fn(key) % capacity;
    const uint32_t start_index = index;
    _hash_map_entry_t* tombstone = NULL;
    while (true) {
        _hash_map_entry_t* _entry = hash_map__at(self, index);
        switch (_entry->type) {
            case HASH_MAP_ENTRY_TYPE_EMPTY: {
                return tombstone ? tombstone : _entry;
            } break ;
            case HASH_MAP_ENTRY_TYPE_TOMBSTONE: {
                if (!tombstone) {
                    tombstone = _entry;
                }
            } break ;
            case HASH_MAP_ENTRY_TYPE_NON_EMPTY: {
                if (self->eq_fn(key, hash_map__internal_entry_to_key(_entry))) {
                    return _entry;
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
    return 0;
}
