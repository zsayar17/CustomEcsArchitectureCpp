#include "ArcheType.hpp"
#include "EntityManager.hpp"
#include "Entity.hpp"
#include <cstring>
#include <algorithm>
#include <map>

void ArcheType::calculateLayout()
{
    entitySize = 0;
    for (size_t i = 0; i < MAX_COMPONENTS; ++i)
    {
        if (signature.test(i))
        {
            componentTypes.push_back(i);
            entitySize += ComponentRegistry::getSize(i);
        }
    }

    size_t availableSpace = CHUNK_SIZE - sizeof(ChunkHeader);
    if (entitySize > 0) {
        chunkCapacity = availableSpace / entitySize;
    } else {
        chunkCapacity = availableSpace;
    }

    size_t currentOffset = sizeof(ChunkHeader);
    for (size_t i = 0; i < componentTypes.size(); ++i)
    {
        ComponentTypeID id = componentTypes[i];
        componentOffsets[id] = currentOffset;
        currentOffset += ComponentRegistry::getSize(id) * chunkCapacity;
    }
}

ArcheType::ArcheType(Signature& sig) : signature(sig)
{
    totalEntities = 0;

    calculateLayout();
}

ArcheType::~ArcheType()
{
    for (size_t i = 0; i < chunks.size(); ++i)
    {
        chunks[i]->~Chunk();
        aligned_free_16(chunks[i]);
    }
}

Chunk* ArcheType::createChunk()
{
    void* mem = aligned_alloc_16(sizeof(Chunk));
    Chunk* chunk = new (mem) Chunk(this, chunkCapacity);
    chunks.push_back(chunk);
    return chunk;
}

Chunk* ArcheType::getFreeChunk() {
    for (size_t i = 0; i < chunks.size(); ++i) {
        if (chunks[i]->header.count < chunks[i]->header.capacity) {
            return chunks[i];
        }
    }
    return createChunk();
}

Entity* ArcheType::getEntity(Chunk* chunk, size_t index)
{
    std::vector<Entity*> entities = EntityManager::getInstace().getEntitiesBySignature(signature);

    for (size_t i = 0; i < entities.size(); i++) {
        if (entities[i]->getIndexInChunk() == index && entities[i]->getChunk() == chunk) {
            return entities[i];
        }
    }
    return NULL;
}

void ArcheType::removeEntity(Entity* entity) {
    Chunk* chunk = entity->getChunk();
    size_t index = entity->getIndexInChunk();
    size_t lastIndex = chunk->header.count - 1;


    if (index != lastIndex) {
        for (size_t i = 0; i < componentTypes.size(); ++i) {
            ComponentTypeID id = componentTypes[i];
            size_t offset = componentOffsets[id];
            size_t componentSize = ComponentRegistry::getSize(id);

            char* dest = chunk->data + offset + index * componentSize;
            char* src = chunk->data + offset + lastIndex * componentSize;
            std::memcpy(dest, src, componentSize);
        }

        Entity* movedEntity = getEntity(chunk, lastIndex);
        if (movedEntity) {
            movedEntity->setIndexInChunk(index);
        }
    }

    (chunk->header.count)--;
    printf("Removed entity from chunk. New count: %zu\n", chunk->header.count);

    if (chunk->header.count == 0) {
        std::vector<Chunk*>::iterator it = std::find(chunks.begin(), chunks.end(), chunk);
        if (it != chunks.end()) {
            chunks.erase(it);
        }
        chunk->~Chunk();
        aligned_free_16(chunk);
    }

    totalEntities--;
    structuralChanged = true;
}

void ArcheType::reAlignChunks() {
    if (chunks.empty() || chunks.size() <= 1) return;

    std::map<std::pair<Chunk*, size_t>, Entity*> entityMap;
    std::vector<Entity*> entities = EntityManager::getInstace().getEntitiesBySignature(signature);

    for (size_t i = 0; i < entities.size(); i++) {
        Entity* entity = entities[i];
        if (entity) {
            entityMap[{entity->getChunk(), entity->getIndexInChunk()}] = entity;
        }
    }

    size_t writeIdx = 0;
    size_t readIdx = chunks.size() - 1;

    while (writeIdx < readIdx) {
        Chunk* writeChunk = chunks[writeIdx];
        Chunk* readChunk = chunks[readIdx];

        if (writeChunk->header.count == writeChunk->header.capacity) {
            writeIdx++;
            continue;
        }

        if (readChunk->header.count == 0) {
            readChunk->~Chunk();
            aligned_free_16(readChunk);
            chunks.pop_back();
            if (readIdx == 0) break;
            readIdx--;
            continue;
        }

        size_t srcIndex = readChunk->header.count - 1;
        size_t destIndex = writeChunk->header.count;

        for (size_t i = 0; i < componentTypes.size(); ++i) {
            ComponentTypeID id = componentTypes[i];
            size_t offset = componentOffsets[id];
            size_t componentSize = ComponentRegistry::getSize(id);

            char* dest = writeChunk->data + offset + destIndex * componentSize;
            char* src = readChunk->data + offset + srcIndex * componentSize;
            std::memcpy(dest, src, componentSize);
        }

        std::map<std::pair<Chunk*, size_t>, Entity*>::iterator it = entityMap.find({readChunk, srcIndex});
        if (it != entityMap.end()) {
            Entity* entity = it->second;
            entity->setChunk(writeChunk);
            entity->setIndexInChunk(destIndex);
        }

        if (readChunk->header.count == 0) {
            readIdx--;
        }

        if (writeChunk->header.count == writeChunk->header.capacity) {
            writeIdx++;
        }
    }

    while (!chunks.empty() && chunks.back()->header.count == 0) {
        Chunk* c = chunks.back();
        c->~Chunk();
        aligned_free_16(c);
        chunks.pop_back();
    }

    structuralChanged = false;
}


std::pair<size_t, Chunk*> ArcheType::addEntity(void** components) {
    Chunk* chunk = getFreeChunk();
    (chunk->header.count)++;
    // printf("%p Adding entity to chunk. New count: %zu\n", chunk, chunk->header.count);

    size_t index = chunk->header.count - 1;
    for (size_t i = 0; i < componentTypes.size(); ++i) {
        ComponentTypeID id = componentTypes[i];
        size_t offset = componentOffsets[id];
        size_t componentSize = ComponentRegistry::getSize(id);

        char* dest = chunk->data + offset + index * componentSize;
        std::memcpy(dest, components[id], componentSize);
    }

    totalEntities++;
    return std::make_pair(index, chunk);
}

void **ArcheType::getComponentData(Entity* entity) {
    void** components = new void*[MAX_COMPONENTS];
    Chunk* chunk = entity->getChunk();
    size_t index = entity->getIndexInChunk();

    for (size_t i = 0; i < componentTypes.size(); ++i) {
        ComponentTypeID id = componentTypes[i];
        size_t offset = componentOffsets[id];
        size_t componentSize = ComponentRegistry::getSize(id);

        char* src = chunk->data + offset + index * componentSize;
        void* compData = malloc(componentSize);
        std::memcpy(compData, src, componentSize);
        components[id] = compData;
    }
    return components;
}

bool ArcheType::isStructuralChanged() const {
    return structuralChanged;
}

std::vector<std::pair<char *, size_t>> ArcheType::getComponentDataByComponentID(ComponentTypeID id) {
    std::vector<std::pair<char*, size_t>> result;

    if (componentOffsets.find(id) == componentOffsets.end()) {
        return result;
    }

    size_t offset = componentOffsets[id];
    // size_t componentSize = ComponentRegistry::getSize(id);

    for (size_t c = 0; c < chunks.size(); ++c) {
        Chunk* chunk = chunks[c];
        char* data = chunk->data + offset;
        
        size_t size = chunk->header.count;
        result.push_back({data, size});
    }

    return result;
}
