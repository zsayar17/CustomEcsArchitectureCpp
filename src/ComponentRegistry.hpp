#ifndef __COMPONENT_REGISTRY_HPP__ 
#define __COMPONENT_REGISTRY_HPP__

#include <vector>
#include <typeinfo>
#include "Types.hpp"

struct ComponentInfo
{
    size_t size;
    void (*deleter)(void*);
};

class ComponentRegistry {
public:
    template <typename T>
    static ComponentTypeID& getTypeID() {
        static ComponentTypeID id = ComponentRegistry::getInfos().size();
        static bool registered = false;
        if (!registered) {
            ComponentInfo info;
            info.size = sizeof(T);
            info.deleter = [](void* ptr) { delete static_cast<T*>(ptr); };
            ComponentRegistry::getInfos().push_back(info);
            registered = true;
        }
        return id;
    }

    static std::vector<ComponentInfo>& getInfos();

    static size_t getSize(ComponentTypeID id);

    static void destroy(ComponentTypeID id, void* ptr);
};

#endif
