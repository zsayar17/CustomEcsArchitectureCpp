#ifndef __ENTITY_HPP__
#define __ENTITY_HPP__

#include "Types.hpp"
#include <map>

class ArcheType;
class Chunk;

class Entity
{
    private:
        EntityID id;
        ArcheType* archeType = NULL;
        Chunk* chunk = NULL;
        size_t indexInChunk = 0;
        
    public:
        Entity(EntityID eid) : id(eid) { }

        EntityID getID() const;
        ArcheType* getArcheType() const;
        size_t getIndexInChunk() const;
        Chunk* getChunk() const;

        void setIndexInChunk(size_t index);
        void setChunk(Chunk* c);

        void subscribeToArcheType(ArcheType* arch, void** components);

        void switchArcheType(ArcheType* newArcheType);

        void createEntityInArcheType(ArcheType* arch, void** components);
};

#endif 