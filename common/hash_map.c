#include "hash_map.h"

#include <assert.h>
#include <string.h>

#include "hash_map_impl.c"

uint32_t hash_map__entry_size(uint32_t size_of_key, uint32_t size_of_value) {
    return sizeof(_hash_map_entry_t) + size_of_key + size_of_value;
}

uint32_t hash_fn__string(const hash_map_key_t* string_key) {
    const char* _string_ky = *(const char**) string_key;
    uint32_t result = 0;
    
    while (*_string_ky != '\0') {
        result += *_string_ky++;
    }

    return result;
}

bool eq_fn__string(const hash_map_key_t* string_key_a, const hash_map_key_t* string_key_b) {
    const char* _string_key_a = *(const char**) string_key_a;
    const char* _string_key_b = *(const char**) string_key_b;

    return strcmp(_string_key_a, _string_key_b) == 0;
}

bool hash_map__create(hash_map_t* self, void* memory, uint64_t memory_size, uint32_t size_of_key, uint32_t size_of_value, uint32_t (*hash_fn)(const hash_map_key_t*), bool (*eq_fn)(const hash_map_key_t*, const hash_map_key_t*)) {
    self->size_of_key = size_of_key;
    self->size_of_value = size_of_value;
    self->memory = memory;
    self->memory_size = memory_size;
    self->fill = 0;
    self->hash_fn = hash_fn;
    self->eq_fn = eq_fn;

    if (hash_map__capacity(self) == 0) {
        return false;
    }

    hash_map__clear(self);

    return true;
}

hash_map_key_t* hash_map__insert(hash_map_t* self, const hash_map_key_t* key, const hash_map_value_t* value) {
    _hash_map_entry_t* _entry = _hash_map__find(self, key);
    if (_entry == NULL) {
        // full
        return NULL;
    }
    if (_entry->type != HASH_MAP_ENTRY_TYPE_NON_EMPTY) {
        ++self->fill;
    }

    hash_map_key_t* found_key = hash_map__internal_entry_to_key(_entry);
    hash_map_value_t* found_value = hash_map__internal_entry_to_value(self, _entry);
    _entry->type = HASH_MAP_ENTRY_TYPE_NON_EMPTY;
    memcpy(found_key, key, self->size_of_key);
    memcpy(found_value, value, self->size_of_value);

    return found_key;
}

bool hash_map__remove(hash_map_t* self, const hash_map_key_t* key) {
    if (self->fill == 0) {
        return false;
    }

    hash_map_value_t* found_value = hash_map__find(self, key);
    if (found_value == NULL) {
        return false;
    }

    _hash_map_entry_t* _entry = hash_map__value_to_internal_entry(self, found_value);
    assert(_entry->type == HASH_MAP_ENTRY_TYPE_NON_EMPTY);

    _entry->type = HASH_MAP_ENTRY_TYPE_TOMBSTONE;
    --self->fill;

    return true;
}

uint32_t hash_map__size(hash_map_t* self) {
    return self->fill;
}

uint32_t hash_map__capacity(hash_map_t* self) {
    return self->memory_size / _hash_map__entry_size(self);
}

void hash_map__clear(hash_map_t* self) {
    self->fill = 0;
    const uint32_t capacity = hash_map__capacity(self);
    for (uint32_t index = 0; index < capacity; ++index) {
        _hash_map_entry_t* _entry = hash_map__at(self, index);
        _entry->type = HASH_MAP_ENTRY_TYPE_EMPTY;
    }
}

hash_map_value_t* hash_map__find(hash_map_t* self, const hash_map_key_t* key) {
    if (self->fill == 0) {
        return NULL;
    }

    _hash_map_entry_t* _entry = _hash_map__find(self, key);
    if (_entry == NULL || _entry->type != HASH_MAP_ENTRY_TYPE_NON_EMPTY) {
        return NULL;
    }

    if (self->eq_fn(key, hash_map__internal_entry_to_key(_entry))) {
        return hash_map__internal_entry_to_value(self, _entry);
    }

    return NULL;
}

hash_map_key_t* hash_map__begin(hash_map_t* self) {
    hash_map_key_t* end = hash_map__end(self);
    _hash_map_entry_t* _end = hash_map__key_to_internal_entry(end);
    _hash_map_entry_t* _entry = self->memory;

    while (_entry != _end) {
        if (_entry->type == HASH_MAP_ENTRY_TYPE_NON_EMPTY) {
            return hash_map__internal_entry_to_key(_entry);
        }
        _entry = (_hash_map_entry_t*) ((char*) _entry + _hash_map__entry_size(self));
    }

    return end;
}

hash_map_key_t* hash_map__next(hash_map_t* self, hash_map_key_t* key) {
    hash_map_key_t* end = hash_map__end(self);
    _hash_map_entry_t* _end = hash_map__key_to_internal_entry(end);
    _hash_map_entry_t* _entry = hash_map__key_to_internal_entry(key);

    while (_entry != _end) {
        _entry = (_hash_map_entry_t*) ((char*) _entry + _hash_map__entry_size(self));
        if (_entry == _end) {
            return end;
        }
        if (_entry->type == HASH_MAP_ENTRY_TYPE_NON_EMPTY) {
            return hash_map__internal_entry_to_key(_entry);
        }
    }

    return end;
}

hash_map_key_t* hash_map__end(hash_map_t* self) {
    _hash_map_entry_t* _end = hash_map__at(self, hash_map__capacity(self));
    return hash_map__internal_entry_to_key(_end);
}

hash_map_value_t* hash_map__value(hash_map_t* self, hash_map_key_t* key) {
    return (hash_map_value_t*) ((char*) key + self->size_of_key);
}

hash_map_key_t* hash_map__key(hash_map_t* self, hash_map_value_t* value) {
    return (hash_map_value_t*) ((char*) value - self->size_of_value);
}
