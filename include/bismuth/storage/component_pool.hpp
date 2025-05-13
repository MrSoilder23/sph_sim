#pragma  once
// C++ standard libraries
#include <cassert>
#include <cstddef>
#include <utility>
#include <vector>
#include <limits>

namespace bismuth {

class ISparseSet {
    public:
        using EntityID = size_t;

        virtual ~ISparseSet() = default;
        virtual void RemoveComponent(const EntityID& entity) = 0;
};

template<typename ComponentType>
class ComponentPool : public ISparseSet{
    public:
        using EntityID = size_t;
        static constexpr size_t INVALID_INDEX = std::numeric_limits<size_t>::max();

        ComponentType& GetComponent(const EntityID& entity) {
            assert(HasComponent(entity) && "No entity with such component");

            return mDenseComponents[mComponentLocation[entity]];
        }
        const std::vector<size_t>& GetDenseEntities() const noexcept{
            return mDenseEntities;
        }

        inline bool HasComponent(const EntityID& entity) const noexcept {
            return entity < mComponentLocation.size() && 
                   mComponentLocation[entity] != INVALID_INDEX;
        }

        template<typename... Args>
        void AddComponent(const EntityID& entity, Args&&... args) {
            if(HasComponent(entity)) {
                mDenseComponents[mComponentLocation[entity]] = ComponentType(std::forward<Args>(args)...);
                return;
            }

            if(entity >= mComponentLocation.size()) {
                mComponentLocation.resize(entity+1, INVALID_INDEX);
            }

            mComponentLocation[entity] = mDenseComponents.size();
            mDenseComponents.push_back(ComponentType(std::forward<Args>(args)...));
            mDenseEntities.push_back(entity);
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

        void RemoveComponent(const EntityID& entity) override {
            const size_t& index = mComponentLocation[entity];
            const size_t& lastEntity = mDenseEntities.back();

            mDenseComponents[index] = std::move(mDenseComponents.back());
            mDenseEntities[index] = lastEntity;
            mComponentLocation[lastEntity] = index;

            mDenseComponents.pop_back();
            mDenseEntities.pop_back();

            mComponentLocation[entity] = INVALID_INDEX;
        }

        inline void Reserve(const size_t& capacity) {
            mComponentLocation.resize(capacity+1, INVALID_INDEX);
            mDenseComponents.reserve(capacity);
            mDenseEntities.reserve(capacity);
        }

        // For efficient reading/sending data to gpu
        const std::vector<ComponentType>& GetDenseComponents() const {
            return mDenseComponents;
        }

        std::vector<ComponentType>::iterator ComponentBegin() {
            return mDenseComponents.begin();
        }
        std::vector<ComponentType>::iterator ComponentEnd() {
            return mDenseComponents.end();
        }

    private:
        std::vector<size_t> mComponentLocation;
        std::vector<ComponentType> mDenseComponents;
        std::vector<size_t> mDenseEntities;
};

}