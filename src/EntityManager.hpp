#ifndef __ENTITYMANAGER_HPP__
#define __ENTITYMANAGER_HPP__

#include "Types.hpp"
#include "ComponentRegistry.hpp"
#include "Entity.hpp"
#include "ArcheType.hpp"
#include "ISystem.hpp"
#include <map>
#include <cstring>
#include <iostream>

class ArcheType;
class Entity;

struct SignatureComparator {
    bool operator()(const Signature& lhs, const Signature& rhs) const {
        return lhs.to_ullong() < rhs.to_ullong();
    }
};

class EntityManager
{
    private:
        EntityID nextEntityID;

        std::map<EntityID, std::pair<Signature, void**>> tempCreateEntitySignatures;
        std::map<Signature, ArcheType*, SignatureComparator> archetypes;
        std::map<Entity*, Signature> changedEntities;
        std::vector<Entity*> removedEntities;

        std::vector<ISystem*> systems;

        std::map<EntityID, Entity*> entities;


        ArcheType* getOrCreateArchetype(Signature sig);
        std::vector<ArcheType*> getArcheTypesBySignature(Signature sig);

        void reAlignArchetypes();

        void clearCreateTempEntites();

        void clearChangedEntities();

        void clearRemovedEntities();

        void applyCreateTempEntities();

        void applyChangedEntities();

        void applyRemovedEntities();

    public:
        static EntityManager& getInstace();

        EntityID CreateEntity();

        void RemoveEntity(EntityID entity);

        template <typename T>
        void AddComponent(EntityID entity, const T& component) {
            tempCreateEntitySignatures[entity].first.set(ComponentRegistry::getTypeID<T>());
            tempCreateEntitySignatures[entity].second[ComponentRegistry::getTypeID<T>()] = new T(component);
        }

        template<typename T>
        void RemoveCompoenent(EntityID entity) {
            if (tempCreateEntitySignatures.find(entity) != tempCreateEntitySignatures.end()) {
                tempCreateEntitySignatures[entity].first.reset(ComponentRegistry::getTypeID<T>());
                if (tempCreateEntitySignatures[entity].second[ComponentRegistry::getTypeID<T>()] != NULL) {
                    ComponentRegistry::destroy(ComponentRegistry::getTypeID<T>(), tempCreateEntitySignatures[entity].second[ComponentRegistry::getTypeID<T>()]);
                    tempCreateEntitySignatures[entity].second[ComponentRegistry::getTypeID<T>()] = NULL;
                }
                return;
            }

            if (entities.find(entity) != entities.end()) {
                Entity* ent = entities[entity];
                Signature sig = ent->getArcheType()->getSignature();
                sig.reset(ComponentRegistry::getTypeID<T>());
                changedEntities[ent] = sig;
            }
        }

        void PlayBackEntities();

        std::vector<Entity*> getEntitiesBySignature(Signature sig);

        void SystemLoop();

        void RegisterSystem(ISystem* system);
        void UnregisterSystem(ISystem* system);

        template<typename... Args>
        std::vector<ArcheType*> getArcheTypes()
        {
            Signature sig;

            int dummy[] = {
                0,
                ( (void)sig.set(ComponentRegistry::getTypeID<Args>()), 0 )...
            };
            (void)dummy;

            return getArcheTypesBySignature(sig);
        }

        Entity* getEntityById(EntityID id);
};


#endif
