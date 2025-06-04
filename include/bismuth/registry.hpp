#pragma once
// C++ standard libraries
#include <bit>
#include <cstddef>
#include <memory>
#include <vector>
#include <array>

// Own libraries
#include "./bismuth/storage/component_pool.hpp"
#include "./bismuth/storage/component_view.hpp"

namespace bismuth {

namespace internal_id_gen {
    inline size_t id = 0;
    
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
            static const size_t type_id = GetPoolID<ComponentName>();

            if (type_id >= mComponentPool.size()) {
                mComponentPool.resize(type_id + 1);
            }
            
            if (!mComponentPool[type_id]) {
                mComponentPool[type_id] = std::make_unique<ComponentPool<ComponentName>>();
            }
            
            return *static_cast<ComponentPool<ComponentName>*>(mComponentPool[type_id].get());
        }

        size_t CreateEntity() {
            const size_t id = mEntities.size();
            mEntities.emplace_back();
            return id;
        }

        template<typename ComponentName>
        void EmplaceComponent(size_t entityID, ComponentName&& component) {
            assert(entityID < mEntities.size() && "Invalid entity ID");
            
            auto& pool = GetComponentPool<ComponentName>();
            const size_t compID = GetPoolID<ComponentName>();
            
            pool.AddComponent(entityID, std::forward<ComponentName>(component));
            mEntities[entityID] |= (1ULL << compID);
        }

        template<typename ComponentName, typename... Args>
        void EmplaceComponent(size_t entityID, Args&&... args) {
            assert(entityID < mEntities.size() && "Invalid entity ID");
            
            auto& pool = GetComponentPool<ComponentName>();
            const size_t compID = GetPoolID<ComponentName>();
            
            pool.AddComponent(entityID, std::forward<Args>(args)...);
            mEntities[entityID] |= (1ULL << compID);
        }

        template<typename ComponentName>
        bool HasComponent(size_t entityID) const {
            if (entityID >= mEntities.size()) return false;
            const size_t compID = GetPoolID<ComponentName>();
            return (mEntities[entityID] >> compID) & 1;
        }

        template<typename ComponentName>
        void RemoveComponent(size_t entityID) {
            if (entityID >= mEntities.size()) return;
            
            auto& pool = GetComponentPool<ComponentName>();
            const size_t compID = GetPoolID<ComponentName>();
            
            pool.RemoveComponent(entityID);
            mEntities[entityID] &= ~(1ULL << compID);
        }

        void RemoveEntity(size_t entityID) {
            if (entityID >= mEntities.size()) return;
            
            uint64_t mask = mEntities[entityID];
            while (mask) {
                const size_t idx = std::countr_zero(mask);
                mComponentPool[idx]->RemoveComponent(entityID);
                mask &= ~(1ULL << idx);
            }
            mEntities[entityID] = 0;
        }

        template<typename... ComponentName>
        ComponentView<ComponentName...> GetView() {
            return ComponentView<ComponentName...>(GetComponentPool<ComponentName>()...);
        }

    private:
        std::vector<size_t> mEntities; // Component bitmask per entity
        std::vector<std::unique_ptr<ISparseSet>> mComponentPool;
};

}