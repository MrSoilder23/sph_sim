#pragma  once
// C++ standard libraries
#include <cassert>
#include <cstddef>
#include <utility>
#include <vector>

class IComponentPool {};

template<typename ComponentType>
class ComponentPool : public IComponentPool{
    public:
        using EntityID = size_t;

        ComponentType& GetComponent(const EntityID& entity) {
            assert(HasComponent(entity) && "No entity with such component");

            const size_t& denseComponentLocation = mComponentLocation[entity];

            return mDenseComponents[denseComponentLocation];
        }
        const std::vector<size_t>& GetDenseEntities() {
            return mDenseEntities;
        }

        inline bool HasComponent(const EntityID& entity) const noexcept {
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
        void RemoveComponent(const EntityID& entity) noexcept {
            if(!HasComponent(entity)) {
                return;
            }

            const size_t& index = mComponentLocation[entity];
            const size_t& lastComponentIndex = mDenseComponents.size() - 1;
            const size_t& lastEntity = mDenseEntities[lastComponentIndex];

            if(index != lastComponentIndex)  {
                mDenseComponents[index] = std::move(mDenseComponents[lastComponentIndex]);
                mDenseEntities[index] = lastEntity;
                mComponentLocation[lastEntity] = index;
            }

            mDenseComponents.pop_back();
            mDenseEntities.pop_back();

            mComponentLocation[entity] = INVALID_INDEX;
        }

        inline void Reserve(const size_t& capacity) {
            mComponentLocation.resize(capacity+1, INVALID_INDEX);
            mDenseComponents.reserve(capacity);
            mDenseEntities.reserve(capacity);
        }    

        std::vector<ComponentType>::iterator ComponentBegin() {
            return mDenseComponents.begin();
        }
        std::vector<ComponentType>::iterator ComponentEnd() {
            return mDenseComponents.end();
        }

    private:
        static constexpr size_t INVALID_INDEX = std::numeric_limits<size_t>::max();

        std::vector<size_t> mComponentLocation;
        std::vector<ComponentType> mDenseComponents;
        std::vector<size_t> mDenseEntities;
};