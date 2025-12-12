#include "Chunk.hpp"

Chunk::Chunk(ArcheType* arch, size_t cap) {
    header.archetype = arch;
    header.count = 0;
    header.capacity = cap;
}
