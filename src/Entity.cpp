#include "Entity.hpp"
#include "ArcheType.hpp"
#include "ComponentRegistry.hpp"

EntityID Entity::getID() const { return id; }
ArcheType* Entity::getArcheType() const { return archeType; }
size_t Entity::getIndexInChunk() const { return indexInChunk; }
Chunk* Entity::getChunk() const { return chunk; }

void Entity::setIndexInChunk(size_t index) { indexInChunk = index; }
void Entity::setChunk(Chunk* c) { chunk = c; }

void Entity::subscribeToArcheType(ArcheType* arch, void** components) {
    if (archeType != NULL)
    {
        archeType->removeEntity(this);
    }
    createEntityInArcheType(arch, components);
}

void Entity::switchArcheType(ArcheType* newArcheType) {
    void** newComponents = NULL;
    if (archeType != NULL)
    {
        newComponents = archeType->getComponentData(this);
        archeType->removeEntity(this);
    }
    createEntityInArcheType(newArcheType, newComponents);

    if (archeType != NULL)
    {
        for (size_t i = 0; i < MAX_COMPONENTS; i++)
        {
            if (newComponents[i] != NULL)
            {
                ComponentRegistry::destroy(i, newComponents[i]);
            }
        }
        delete[] newComponents;
    }
}

void Entity::createEntityInArcheType(ArcheType* arch, void** components) {
    if (arch != NULL) 
    {
        std::pair<size_t, Chunk*> result = arch->addEntity(components);
        indexInChunk = result.first;
        chunk = result.second;
    }
    archeType = arch;
}
