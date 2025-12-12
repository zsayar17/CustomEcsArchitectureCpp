#include <iostream>
#include <vector>
#include <chrono>
#include <random>
#include "EntityManager.hpp"
#include "Query.hpp"
#include "AJob.hpp"

const int ENTITY_COUNT = 1000000;
const int ITERATIONS = 10;

struct CompPos {
    float x, y, z;
};
struct CompVel {
    float vx, vy, vz;
};

struct CompTag  {};
struct CompTag2 {};
struct CompTag3 {};
struct CompTag4 {};



class BaseObject {
public:
    virtual void update() = 0;
    virtual ~BaseObject() {}
};
class ClassicObject : public BaseObject {
public:
    float x, y, z;
    float vx, vy, vz;

    virtual void update() {
        x += sin(vx) * cos(0);
        y += sin(vy) * cos(0);
        z += sin(vz) * cos(0);
    }
};

template<typename T>
class MyECSJob : public AJob<CompPos, CompVel, T> {
    public:
        MyECSJob() : AJob<CompPos, CompVel, T>() { }

        void execute(size_t index) {
            std::pair<char*, size_t> posData = this->template getComponentDataInIndex<CompPos>(index);
            std::pair<char*, size_t> velData = this->template getComponentDataInIndex<CompVel>(index);

            CompPos* positions = (CompPos*)posData.first;
            CompVel* velocities = (CompVel*)velData.first;
            size_t count = posData.second;

            for (size_t i = 0; i < count; ++i) {
                positions[i].x += sin(velocities[i].vx) * cos(0);
                positions[i].y += sin(velocities[i].vy) * cos(0);
                positions[i].z += sin(velocities[i].vz) * cos(0);
            }
        }
};

template<typename T>
class MyECSJobSystem : public ISystem {
    public:
        void Update()  override {
            MyECSJob<T> job;
            job.scheduleParallel();
        }
};

template<typename T>
class MyECSSystem : public ISystem {
    public:
        void Update() override {
            Query<CompPos, CompVel, T> query;

            std::vector<std::pair<char*, size_t>> posData = query.template getComponentData<CompPos>();
            std::vector<std::pair<char*, size_t>> velData = query.template getComponentData<CompVel>();

            for (size_t c = 0; c < posData.size(); ++c) {
                CompPos* positions = (CompPos*)posData[c].first;
                CompVel* velocities = (CompVel*)velData[c].first;
                size_t count = posData[c].second;

                for (size_t i = 0; i < count; ++i) {
                    positions[i].x += sin(velocities[i].vx) * cos(0);
                    positions[i].y += sin(velocities[i].vy) * cos(0);
                    positions[i].z += sin(velocities[i].vz) * cos(0);
                }
            }
        }
};


std::vector<BaseObject*> createOOPObjects(int count) {
    // std::cout << "\n[Prep] OOP objects are creating (Heap Allocation)..." << std::endl;
    std::vector<BaseObject*> objects;

    std::vector<char*> randomMemoryBlocks;
    std::mt19937 rng(42);
    std::uniform_int_distribution<int> dist(100, 10000); // 100 B – 10 KB

    std::chrono::high_resolution_clock::time_point start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < count; ++i) {

        ClassicObject* obj = new ClassicObject();
        obj->x = 0; obj->y = 0; obj->z = 0;
        obj->vx = 1.0f; obj->vy = 1.0f; obj->vz = 1.0f;
        objects.push_back(obj);

        int randomSize = dist(rng);
        char* block = new char[randomSize];
        randomMemoryBlocks.push_back(block);
    }

    std::chrono::high_resolution_clock::time_point end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> diff = end - start;
    // std::cout << "OOP Object Creation Timing: " << diff.count() << " seconds" << std::endl;

    for (size_t i = 0; i < randomMemoryBlocks.size(); ++i) {
        delete[] randomMemoryBlocks[i];
    }
    randomMemoryBlocks.clear();
    return objects;
}

template<typename T>
std::vector<EntityID> createEntities(size_t count)
{
    // std::cout << "\n[Prep] ECS entities are being created..." << std::endl;
    EntityManager& entityManager = EntityManager::getInstace();

    std::vector<EntityID> entityIDs;
    EntityID entity;

    std::chrono::high_resolution_clock::time_point start = std::chrono::high_resolution_clock::now();
    for (size_t i = 0; i < count; ++i) {
        entity = entityManager.CreateEntity();
        entityManager.AddComponent(entity, CompPos{0, 0, 0});
        entityManager.AddComponent(entity, CompVel{1.0f, 1.0f, 1.0f});
        entityManager.AddComponent(entity, T());

        entityIDs.push_back(entity);
    }
    entityManager.PlayBackEntities();
    std::chrono::high_resolution_clock::time_point end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> diff = end - start;
    // std::cout << "ECS Entity Creation Timing: " << diff.count() << " seconds" << std::endl;

    return entityIDs;
}



std::chrono::duration<double> measureOOPUpdate(std::vector<std::vector<BaseObject*>>& objects) {
    // std::cout << "[Test] OOP Update loop is getting started..." << std::endl;
    std::chrono::high_resolution_clock::time_point start = std::chrono::high_resolution_clock::now();

    for (size_t i = 0; i < objects.size(); ++i) {
        std::vector<BaseObject*>& objGroup = objects[i];
        for (size_t it = 0; it < ITERATIONS; ++it) {
            for (size_t j = 0; j < objGroup.size(); ++j) {
                objGroup[j]->update();
            }
        }
    }


    std::chrono::high_resolution_clock::time_point end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> diffOOP = end - start;
    std::cout << "OOP Timing: " << diffOOP.count() << " seconds. Timing per iteration: " << diffOOP.count() / ITERATIONS << " seconds." << std::endl;

    // for (size_t i = 0; i < objects.size(); ++i) {
    //     cleanupOOPObjects(objects[i]);
    // }
    return diffOOP;
}
std::chrono::duration<double> mesureSystem(std::vector<ISystem*> system, std::string name) {
    EntityManager& entityManager = EntityManager::getInstace();
    for (size_t i = 0; i < system.size(); ++i) {
        entityManager.RegisterSystem(system[i]);
    }

    // std::cout << "[Test] " << name << " Update loop is getting started..." << std::endl;
    std::chrono::high_resolution_clock::time_point startECS = std::chrono::high_resolution_clock::now();

    for (int it = 0; it < ITERATIONS; ++it) {
        entityManager.SystemLoop();
    }
    std::chrono::high_resolution_clock::time_point endECS = std::chrono::high_resolution_clock::now();

    for (size_t i = 0; i < system.size(); ++i) {
        entityManager.UnregisterSystem(system[i]);
    }

    std::chrono::duration<double> duration = endECS - startECS;
    std::cout << name << " Timing: " << duration.count() << " seconds. Timing per iteration: " << duration.count() / ITERATIONS << " seconds." << std::endl ;

    for (size_t i = 0; i < system.size(); ++i) {
        delete system[i];
    }

    return endECS - startECS;
}

void cleanupOOPObjects(std::vector<BaseObject*>& objects) {
    for (size_t i = 0; i < objects.size(); ++i) {
        delete objects[i];
    }
    objects.clear();
}

void validateDataIntegrity(const std::vector<EntityID>& entityIDs, const std::vector<BaseObject*>& objects) {
    EntityManager& entityManager = EntityManager::getInstace();
    std::cout << "\n[Validation] Comparing ECS and OOP data integrity..." << std::endl;
    for (size_t i = 0; i < entityIDs.size(); ++i) {
        Entity* ent = entityManager.getEntityById(entityIDs[i]);
        CompPos* pos2 = reinterpret_cast<CompPos*>(ent->getArcheType()->getComponentData(ent)[ComponentRegistry::getTypeID<CompPos>()]);

        if (pos2->x != ((ClassicObject*)objects[i])->x ||
            pos2->y != ((ClassicObject*)objects[i])->y ||
            pos2->z != ((ClassicObject*)objects[i])->z) {
            std::cout << "Data mismatch for entity ID " << entityIDs[i] << std::endl;

            std::cout << "ECS Position: (" << pos2->x << ", " << pos2->y << ", " << pos2->z << ")" << std::endl;
            std::cout << "OOP Position: (" << ((ClassicObject*)objects[i])->x << ", " << ((ClassicObject*)objects[i])->y << ", " << ((ClassicObject*)objects[i])->z << ")" << std::endl;
            return;
        }
    }
    std::cout << "Data integrity validated: ECS and OOP results match!" << std::endl;
}
void CompareTimeDifferences(std::string name1, std::chrono::duration<double> diff1, std::string name2, std::chrono::duration<double> diff2) {
    double time1 = diff1.count();
    double time2 = diff2.count();

    if (time1 < time2) {
        double  faster = time2 / time1;
        std::cout << name1 << " is faster than " << name2 << " by " << faster << " times." << std::endl;
    } else {
        double  faster = time1 / time2;
        std::cout << name2 << " is faster than " << name1 << " by " << faster << " times." << std::endl;
    }
}

int main() {
    int totalSystems = 4;
    std::cout << "=== ECS vs OOP Benchmark (" << ENTITY_COUNT * totalSystems << " entities) (" << ITERATIONS << " iterations) ===" << std::endl;

    std::vector<BaseObject*> objects = createOOPObjects(ENTITY_COUNT);
    std::vector<EntityID> entityIDs = createEntities<CompTag>(ENTITY_COUNT);

    std::vector<EntityID> moreEntityIDs = createEntities<CompTag2>(ENTITY_COUNT);
    std::vector<BaseObject*> moreObjects = createOOPObjects(ENTITY_COUNT);

    std::vector<BaseObject*> evenMoreObjects = createOOPObjects(ENTITY_COUNT);
    std::vector<EntityID> evenMoreEntityIDs = createEntities<CompTag3>(ENTITY_COUNT);

    std::vector<EntityID> lastEntityIDs = createEntities<CompTag4>(ENTITY_COUNT);
    std::vector<BaseObject*> lastObjects = createOOPObjects(ENTITY_COUNT);

    std::vector<std::vector<BaseObject*>> oopObjectGroups = {objects, moreObjects, evenMoreObjects, lastObjects};
    std::vector<ISystem*> ecsSystems = {new MyECSSystem<CompTag>(), new MyECSSystem<CompTag2>(), new MyECSSystem<CompTag3>(), new MyECSSystem<CompTag4>()};
    std::vector<ISystem*> jobSystems = {new MyECSJobSystem<CompTag>(), new MyECSJobSystem<CompTag2>(), new MyECSJobSystem<CompTag3>(), new MyECSJobSystem<CompTag4>()};

    std::chrono::duration<double> diffOOP = measureOOPUpdate(oopObjectGroups);
    std::chrono::duration<double> diffECS = mesureSystem(ecsSystems, "ECSSystem");
    std::chrono::duration<double> diffECSJob = mesureSystem(jobSystems, "JobSystem");


    // validateDataIntegrity(entityIDs, objects);
    // validateDataIntegrity(moreEntityIDs, moreObjects);
    // validateDataIntegrity(evenMoreEntityIDs, evenMoreObjects);
    // validateDataIntegrity(lastEntityIDs, lastObjects);


    std::cout << "\n=== Benchmark Results Comparison ===" << std::endl;
    CompareTimeDifferences("OOP", diffOOP, "ECSSystem", diffECS);
    CompareTimeDifferences("OOP", diffOOP, "JobSystem", diffECSJob);
    CompareTimeDifferences("ECSSystem", diffECS, "JobSystem", diffECSJob);

    return 0;
}
