#pragma once
// C++ standard libraries
#include <cstddef>
#include <memory>
#include <vector>

// Own libraries
#include "./bismuth/storage/component_pool.hpp"
#include "./bismuth/storage/component_view.hpp"

namespace bismuth {

namespace internal_id_gen {
    static size_t id = 0;
    
    template<typename>
    size_t generate_id() {
        static size_t componentID = id++;
        return componentID;
    }
}

class Registry {
    public:
        template<typename ComponentName>
        inline size_t GetPoolID() const {
            static size_t componentID = internal_id_gen::generate_id<ComponentName>();
            return componentID;
        }

        
        template<typename ComponentName>
        ComponentPool<ComponentName>& GetComponentPool() {
            static uint32_t type_id = GetPoolID<ComponentName>();

            if (type_id >= mComponentPool.size()) {
                mComponentPool.resize(type_id + 1);
            }
            
            if (!mComponentPool[type_id]) {
                mComponentPool[type_id] = std::make_unique<ComponentPool<ComponentName>>();
            }
            
            return *static_cast<ComponentPool<ComponentName>*>(mComponentPool[type_id].get());
        }


        size_t CreateEntity() {
            mEntities.push_back(false);
            const size_t& currentEntityAmount = mEntities.size()-1;

            return currentEntityAmount;
        }

        template<typename ComponentName>
        void EmplaceComponent(const size_t& entityID, ComponentName& component) {
            static ComponentPool<ComponentName>& pool = GetComponentPool<ComponentName>();
            
            pool.AddComponent(entityID, component);
            mEntities[entityID] = true;
        }
        template<typename ComponentName, typename... Args>
        void EmplaceComponent(const size_t& entityID, Args&&... args) {
            static ComponentPool<ComponentName>& pool = GetComponentPool<ComponentName>();
            
            pool.AddComponent(entityID, args...);
            mEntities[entityID] = true;
        }

        template<typename ComponentName>
        bool HasComponent(const size_t& entityID) {
            return GetComponentPool<ComponentName>().HasComponent(entityID);
        }

        template<typename ComponentName>
        void RemoveComponent(const size_t& entityID) {
            static ComponentPool<ComponentName>& pool = GetComponentPool<ComponentName>();

            pool.RemoveComponent(entityID);
        }

        void RemoveEntity(const size_t& entityID) {
            if(entityID >= mEntities.size()) {
                return;
            }

            for(auto& pool : mComponentPool) {
                pool->RemoveComponent(entityID);
            }
            mEntities[entityID] = false;
        }

        template<typename... ComponentName>
        ComponentView<ComponentName...> GetView() {
            return ComponentView<ComponentName...>(GetComponentPool<ComponentName>()...);
        }

    private:
        std::vector<bool> mEntities;
        std::vector<std::unique_ptr<ISparseSet>> mComponentPool;
};

}