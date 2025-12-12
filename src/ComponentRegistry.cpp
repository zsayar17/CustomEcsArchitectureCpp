#include "ComponentRegistry.hpp"

std::vector<ComponentInfo>& ComponentRegistry::getInfos() {
    static std::vector<ComponentInfo> infos;
    return infos;
}

size_t ComponentRegistry::getSize(ComponentTypeID id) {
    return getInfos()[id].size;
}

void ComponentRegistry::destroy(ComponentTypeID id, void* ptr) {
    if (id < getInfos().size() && getInfos()[id].deleter) {
        getInfos()[id].deleter(ptr);
    }
}
