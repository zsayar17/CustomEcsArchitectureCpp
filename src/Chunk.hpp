#ifndef __CHUNK_HPP__
#define __CHUNK_HPP__

#include "Types.hpp"

class ArcheType;

struct ChunkHeader {
    ArcheType* archetype;
    size_t count;
    size_t capacity;

    char padding[16 - (sizeof(ArcheType*) + 2 * sizeof(size_t)) % 16]; 
};


class Chunk {
public:
    union {
        char data[CHUNK_SIZE];
        ChunkHeader header;
    };
    
    Chunk(ArcheType* arch, size_t cap);
};

#endif