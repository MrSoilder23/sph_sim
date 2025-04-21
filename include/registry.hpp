#pragma once
// C++ standard libraries
#include <cstddef>
#include <memory>
#include <vector>

// Own libraries
#include "./storage/component_pool.hpp"

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
            auto& pool = GetComponentPool<ComponentName>();
            
            pool.AddComponent(entityID, component);
        }

        template<typename ComponentName>
        bool HasComponent(const size_t& entityID) {
            static size_t type_id = GetPoolID<ComponentName>();

            return entityID < mEntities.size() &&
                   GetComponentPool<ComponentName>().HasComponent(entityID);
        }

    private:
        std::vector<bool> mEntities;
        std::vector<std::unique_ptr<IComponentPool>> mComponentPool;
};