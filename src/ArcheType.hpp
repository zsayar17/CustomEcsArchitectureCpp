#ifndef __ARCHETYPE_HPP__
#define __ARCHETYPE_HPP__

#include "Types.hpp"
#include "ComponentRegistry.hpp"
#include "Chunk.hpp"
#include "Memory.hpp"

class Entity;
class EntityManager;
class Chunk;

class ArcheType
{

private:
    std::vector<Chunk*> chunks;
    Signature signature;
    std::vector<ComponentTypeID> componentTypes;
    std::map<ComponentTypeID, size_t> componentOffsets;
    size_t entitySize;
    size_t chunkCapacity;
    bool structuralChanged = false;
    size_t totalEntities = 0;

    void calculateLayout();

    Entity* getEntity(Chunk* chunk, size_t index);


public:
    ArcheType(Signature& sig);

    ~ArcheType();

    Signature getSignature() const { return signature; }
    const std::vector<Chunk*>& getChunks() const { return chunks; }
    size_t getChunkCapacity() const { return chunkCapacity; }
    size_t getTotalEntities() const { return totalEntities; }

    Chunk* createChunk();

    Chunk* getFreeChunk();

    void removeEntity(Entity* entity);

    void reAlignChunks();

    std::pair<size_t, Chunk*> addEntity(void** components);

    void **getComponentData(Entity* entity);

    bool isStructuralChanged() const;

    std::vector<std::pair<char *, size_t>> getComponentDataByComponentID(ComponentTypeID id);
};


#endif
