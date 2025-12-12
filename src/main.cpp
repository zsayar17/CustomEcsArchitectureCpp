#include <iostream>
#include "EntityManager.hpp"
#include "Query.hpp"

struct MyComponentA
{
    int value;
};

struct MyComponentB
{
    float value;
};

struct MyComponentC
{
    float x, y, z, w;
};

class MySystem : public ISystem
{
    void Update()
    {
        Query<MyComponentA, MyComponentB, MyComponentC> query;

        std::vector<std::pair<char*, size_t>> compAData = query.getComponentData<MyComponentA>();
        std::vector<std::pair<char*, size_t>> compBData = query.getComponentData<MyComponentB>();

        for (size_t i = 0; i < compAData.size(); i++)
        {
            MyComponentA* compA = reinterpret_cast<MyComponentA*>(compAData[i].first);
            MyComponentB* compB = reinterpret_cast<MyComponentB*>(compBData[i].first);

            size_t count = compAData[i].second;

            for (size_t j = 0; j < count; j++)
            {
                std::cout << compA[j].value << ", " << compB[j].value << std::endl;
            }
        }
    }
};


int main()
{
    EntityManager& entityManager = EntityManager::getInstace();

    /*EntityID entity = entityManager.CreateEntity();
    std::cout << "Created Entity with ID: " << entity << std::endl;
    EntityID entity2 = entityManager.CreateEntity();
    std::cout << "Created Entity with ID: " << entity2 << std::endl;
    EntityID entity3 = entityManager.CreateEntity();
    std::cout << "Created Entity with ID: " << entity3 << std::endl;
    EntityID entity4 = entityManager.CreateEntity();
    std::cout << "Created Entity with ID: " << entity4 << std::endl;
    EntityID entity5 = entityManager.CreateEntity();
    std::cout << "Created Entity with ID: " << entity5 << std::endl;

    entityManager.AddComponent(entity, MyComponentA{ 42 });
    entityManager.AddComponent(entity2, MyComponentA{ 100 });
    entityManager.AddComponent(entity3, MyComponentA{ 7 });
    entityManager.AddComponent(entity4, MyComponentA{ 7 });

    std::cout << "Added MyComponentA to entities." << std::endl;


    entityManager.AddComponent(entity, MyComponentB{ 3.14f });
    entityManager.AddComponent(entity2, MyComponentB{ 6.28f });
    entityManager.AddComponent(entity3, MyComponentB{ 1.61f });
    entityManager.AddComponent(entity4, MyComponentA{ 7 });

    std::cout << "Added MyComponentB to entities." << std::endl;

    entityManager.RemoveCompoenent<MyComponentA>(entity);

    std::cout << "Removed MyComponentA from entity " << entity << std::endl;


    entityManager.AddComponent(entity, MyComponentC{ 1.0f, 2.0f, 3.0f, 4.0f });
    entityManager.AddComponent(entity2, MyComponentC{ 5.0f, 6.0f, 7.0f, 8.0f });

    std::cout << "Added MyComponentC to entities." << std::endl;

    entityManager.RemoveEntity(entity2);
    entityManager.RemoveEntity(entity3);

    std::cout << "Marked entity " << entity2 << " and " << entity3 << " for removal." << std::endl;

    entityManager.PlayBackEntities();

        std::cout << "Applied all entity changes." << std::endl;

    entityManager.RemoveEntity(entity);

    entityManager.RemoveEntity(entity5);*/


    EntityID entity = entityManager.CreateEntity();
    EntityID entity2 = entityManager.CreateEntity();

    entityManager.AddComponent(entity, MyComponentA{ 10 });
    entityManager.AddComponent(entity2, MyComponentA{ 20 });
    entityManager.AddComponent(entity, MyComponentB{ 1.5f });
    entityManager.AddComponent(entity2, MyComponentB{ 2.5f });
    entityManager.AddComponent(entity, MyComponentC{ 0.1f, 0.2f, 0.3f, 0.4f });
    entityManager.PlayBackEntities();

    entityManager.RemoveCompoenent<MyComponentA>(entity2);
    entityManager.PlayBackEntities();

    MySystem system;
    entityManager.RegisterSystem(&system);

    entityManager.SystemLoop();
}
