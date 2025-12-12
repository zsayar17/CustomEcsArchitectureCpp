#include "EntityManager.hpp"
#include "ArcheType.hpp"
#include "Entity.hpp"
#include "ComponentRegistry.hpp"
#include <cstring>
#include <algorithm>

ArcheType* EntityManager::getOrCreateArchetype(Signature sig) {
    if (archetypes.find(sig) != archetypes.end()) {
        return archetypes[sig];
    }

    ArcheType* arch = new ArcheType(sig);
    archetypes[sig] = arch;
    return arch;
}


std::vector<ArcheType*>  EntityManager::getArcheTypesBySignature(Signature sig) {
    std::vector<ArcheType*> result;
    std::map<Signature, ArcheType*, SignatureComparator>::iterator iterator;

    for (iterator = archetypes.begin(); iterator != archetypes.end(); ++iterator) {
        ArcheType* arch = iterator->second;
        if ((arch->getSignature() & sig) == sig) {
            result.push_back(arch);
        }
    }
    return result;
}

void EntityManager::reAlignArchetypes() {
    std::map<Signature, ArcheType*, SignatureComparator>::iterator iterator;
    for (iterator = archetypes.begin(); iterator != archetypes.end(); ++iterator)
    {
        ArcheType* arch = iterator->second;
        if (arch->isStructuralChanged()) {
            arch->reAlignChunks();
        }
    }
}

void EntityManager::clearCreateTempEntites() {
    std::map<EntityID, std::pair<Signature, void**>>::iterator iterator;
    for (iterator = tempCreateEntitySignatures.begin(); iterator != tempCreateEntitySignatures.end(); ++iterator)
    {
        if (iterator->second.second != NULL)
        {
            void** components = iterator->second.second;
            for (size_t i = 0; i < MAX_COMPONENTS; i++)
            {
                if (components[i] != NULL)
                {
                    ComponentRegistry::destroy(i, components[i]);
                }
            }
            delete[] components;
        }
    }

    tempCreateEntitySignatures.clear();
}

void EntityManager::clearChangedEntities() {
    changedEntities.clear();
}

void EntityManager::clearRemovedEntities() {
    removedEntities.clear();
}

void EntityManager::applyCreateTempEntities() {
    std::map<EntityID, std::pair<Signature, void**>>::iterator iterator;
    for (iterator = tempCreateEntitySignatures.begin(); iterator != tempCreateEntitySignatures.end(); ++iterator)
    {
        ArcheType* archetype = getOrCreateArchetype(iterator->second.first);

        Entity* entity;
        if (entities.find(iterator->first) == entities.end()) {
            entity = new Entity(iterator->first);
            entities[iterator->first] = entity;
        }
        else {
            entity = entities[iterator->first];
        }

        entity->subscribeToArcheType(archetype, iterator->second.second);
    }
}

void EntityManager::applyChangedEntities()
{
    for (size_t i = 0; i < removedEntities.size(); i++) {
        if (changedEntities.find(removedEntities[i]) != changedEntities.end()) {
            changedEntities.erase(removedEntities[i]);
        }
    }


    std::map<EntityID, std::pair<Signature, void**>>::iterator it;
    for (it = tempCreateEntitySignatures.begin(); it != tempCreateEntitySignatures.end(); ++it) {
        if (changedEntities.find(entities[it->first]) != changedEntities.end()) {
            void** components = it->second.second;

            Signature changedSizes = changedEntities[entities[it->first]];
            it->second.first = it->second.first & ~changedSizes;
            for (size_t j = 0; j < MAX_COMPONENTS; j++)
            {
                if (changedSizes.test(j) && components[j] != NULL)
                {
                    ComponentRegistry::destroy(j, components[j]);
                    components[j] = NULL;
                }
            }
            changedEntities.erase(entities[it->first]);

        }
    }

    std::map<Entity*, Signature>::iterator iterator;
    for (iterator = changedEntities.begin(); iterator != changedEntities.end(); ++iterator)
    {
        ArcheType* archetype = getOrCreateArchetype(iterator->second);
        Entity* entity = iterator->first;
        entity->switchArcheType(archetype);
    }
}

void EntityManager::applyRemovedEntities() {
    for (size_t i = 0; i < removedEntities.size(); i++) {
        if (tempCreateEntitySignatures.find(removedEntities[i]->getID()) != tempCreateEntitySignatures.end()) {
            void** components = tempCreateEntitySignatures[removedEntities[i]->getID()].second;
            for (size_t j = 0; j < MAX_COMPONENTS; j++)
            {
                if (components[j] != NULL)
                {
                    ComponentRegistry::destroy(j, components[j]);
                }
            }
            delete[] components;
            tempCreateEntitySignatures.erase(removedEntities[i]->getID());
            removedEntities.erase(removedEntities.begin() + i);
            i--;
        }
    }

    for (size_t i = 0; changedEntities.size() > 0 && i < removedEntities.size(); i++) {
        if (changedEntities.find(removedEntities[i]) != changedEntities.end()) {
            changedEntities.erase(removedEntities[i]);
        }
    }

    for (size_t i = 0; i < removedEntities.size(); i++) {
        Entity* entity = removedEntities[i];
        entity->getArcheType()->removeEntity(entity);
        delete entity;
    }
}

EntityManager& EntityManager::getInstace() {
    static EntityManager instance;
    return instance;
}

EntityID EntityManager::CreateEntity() {
    EntityID id = nextEntityID++;

    void** components = new void*[MAX_COMPONENTS];
    memset(components, 0, sizeof(void*) * MAX_COMPONENTS);

    tempCreateEntitySignatures[id] = std::make_pair(0, components);
    return id;
}

void EntityManager::RemoveEntity(EntityID entity) {
    if (tempCreateEntitySignatures.find(entity) != tempCreateEntitySignatures.end()) {
        void** components = tempCreateEntitySignatures[entity].second;
        for (size_t i = 0; i < MAX_COMPONENTS; i++) {
            if (components[i] != NULL) {
                ComponentRegistry::destroy(i, components[i]);
            }
        }
        delete[] components;
        tempCreateEntitySignatures.erase(entity);
        return;
    }

    bool foundInRemoved = false;

    for (size_t i = 0; i < removedEntities.size(); i++) {
        if (removedEntities[i]->getID() == entity) {
            foundInRemoved = true;
            break;
        }
    }

    if (!foundInRemoved) {
        if (entities.find(entity) != entities.end()) {
            removedEntities.push_back(entities[entity]);
        }
    }

}

void EntityManager::PlayBackEntities() {
    if (removedEntities.size() > 0)
    {
        applyRemovedEntities();
        clearRemovedEntities();
    }

    if (changedEntities.size() > 0)
    {
        applyChangedEntities();
        clearChangedEntities();
    }

    if (tempCreateEntitySignatures.size() > 0)
    {
        applyCreateTempEntities();
        clearCreateTempEntites();
    }

    reAlignArchetypes();
}

std::vector<Entity*> EntityManager::getEntitiesBySignature(Signature sig) {
    std::vector<Entity*> result;
    std::map<EntityID, Entity*>::iterator iterator;

    for (iterator = entities.begin(); iterator != entities.end(); ++iterator) {
        Entity* entity = iterator->second;
        if ((entity->getArcheType()->getSignature() & sig) == sig) {
            result.push_back(entity);
        }
    }
    return result;
}

void EntityManager::SystemLoop() {

    for (size_t i = 0; i < systems.size(); i++) {
        systems[i]->Update();
    }
    PlayBackEntities();
}

void EntityManager::RegisterSystem(ISystem* system) {
    systems.push_back(system);
}

void EntityManager::UnregisterSystem(ISystem* system) {
    std::vector<ISystem*>::iterator it = std::find(systems.begin(), systems.end(), system);
    if (it != systems.end()) {
        systems.erase(it);
    }
}

Entity* EntityManager::getEntityById(EntityID id) {
    if (entities.find(id) != entities.end()) {
        return entities[id];
    }
    return NULL;
}
