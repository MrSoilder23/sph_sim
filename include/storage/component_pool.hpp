#pragma  once
// C++ standard libraries
#include <cassert>
#include <cstddef>
#include <utility>
#include <vector>

template<typename ComponentType>
class ComponentPool {
    public:
        using EntityID = size_t;

        ComponentType& GetComponent(const EntityID& entity) {
            assert(HasComponent(entity));

            const size_t& denseComponentLocation = mComponentLocation[entity];

            return mDenseComponents[denseComponentLocation];
        }
        bool HasComponent(const EntityID& entity) {
            return entity < mComponentLocation.size() && 
                   mComponentLocation[entity] != INVALID_INDEX;
        }

        void AddComponent(const EntityID& entity, ComponentType& component) {
            if(HasComponent(entity)) {
                mDenseComponents[mComponentLocation[entity]] = std::move(component);
                return;
            }

            if(entity >= mComponentLocation.size()) {
                mComponentLocation.resize(entity+1, INVALID_INDEX);
            }

            mComponentLocation[entity] = mDenseComponents.size();
            mDenseComponents.push_back(std::move(component));
            mDenseEntities.push_back(entity);
        }
        void RemoveComponent(const EntityID& entity) {
            if(!HasComponent(entity)) {
                return;
            }

            const size_t& index = mComponentLocation[entity];
            const size_t& lastComponentIndex = mDenseComponents.size() - 1;
            const size_t& lastEntity = mDenseEntities[lastComponentIndex];

            std::swap(mDenseComponents[index], mDenseComponents[lastComponentIndex]);
            std::swap(mDenseEntities[index], mDenseEntities[lastComponentIndex]);

            mDenseComponents.pop_back();
            mDenseEntities.pop_back();
        }

        void Reserve(const size_t& capacity) {
            mComponentLocation.resize(capacity+1, INVALID_INDEX);
            mDenseComponents.reserve(capacity);
            mDenseEntities.reserve(capacity);
        }    

    private:
        static constexpr size_t INVALID_INDEX = std::numeric_limits<size_t>::max();

        std::vector<size_t> mComponentLocation;
        std::vector<ComponentType> mDenseComponents;
        std::vector<size_t> mDenseEntities;
};