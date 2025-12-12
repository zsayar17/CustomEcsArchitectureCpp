#ifndef __QUERY_HPP__
#define __QUERY_HPP__

#include "EntityManager.hpp"
#include <vector>
#include <map>
#include <utility>



template<typename... Args>
class Query
{
    std::vector<ArcheType*> archetypes;
    std::vector<size_t> componentSizes;
    size_t totalEntities;

    std::map<ComponentTypeID, std::vector<std::pair<char*, size_t>>> componentDataMap;

    public:
        Query() {
            Signature sig;

            int dummy[] = {
                0,
                ( (void)sig.set(ComponentRegistry::getTypeID<Args>()), 0 )...
            };
            (void)dummy;

            archetypes = EntityManager::getInstace().getArcheTypes<Args...>();

            for (size_t i = 0; i < archetypes.size(); i++) {
                ArcheType* arch = archetypes[i];

                totalEntities += arch->getTotalEntities();
                componentSizes.push_back(arch->getTotalEntities());
            }

            for (size_t i = 0; i < sizeof(sig); i++) {
                if (sig.test(i)) {
                    ComponentTypeID id = i;
                    std::vector<std::pair<char*, size_t>> componentData;

                    for (size_t j = 0; j < archetypes.size(); j++) {
                        ArcheType* arch = archetypes[j];
                        std::vector<std::pair<char*, size_t>> dataParts = arch->getComponentDataByComponentID(id);
                        componentData.insert(componentData.end(), dataParts.begin(), dataParts.end());
                    }
                
                    componentDataMap[id] = componentData;
                }
            }
        }

        template<typename T>
        std::vector<std::pair<char*, size_t>> getComponentData() {
            ComponentTypeID id = ComponentRegistry::getTypeID<T>();
            
            // std::cout << "Query found " << componentDataMap[id].size() << " data parts for component ID " << id << std::endl;
            return componentDataMap[id];
        }

        template<typename T>
        std::pair<char*, size_t> getComponentDataInIndex(size_t index) {
            ComponentTypeID id = ComponentRegistry::getTypeID<T>();
            std::vector<std::pair<char*, size_t>> dataParts = componentDataMap[id];


            if (index >= dataParts.size()) {
                return {NULL, 0};
            }
            
            // std::cout << "Query::getComponentDataInIndex: Retrieved data part for component ID " << id << " at index " << index << std::endl;
            return dataParts[index];
        }

        size_t getTotalEntities() const { return totalEntities; }

        size_t getTotalChunkCount() const {
            componentDataMap.begin();

            if (componentDataMap.empty()) {
                return 0;
            }

            return componentDataMap.begin()->second.size();
        }
};


#endif
