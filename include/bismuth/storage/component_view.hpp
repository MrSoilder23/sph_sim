#pragma once
// C++ standard libraries
#include <algorithm>
#include <array>
#include <cstddef>
#include <iterator>
#include <tuple>
#include <vector>
#include <cstdint>

// Own libraries
#include "./bismuth/storage/component_pool.hpp"

namespace bismuth {

template<typename... ComponentName>
class ComponentView {
    static_assert(sizeof...(ComponentName) > 0, "ComponentView requires at least one component type");

    public:

        ComponentView(ComponentPool<ComponentName>&... componentPool) : mComponentPools(componentPool...) {
            std::array<const std::vector<uint32_t>*, sizeof...(ComponentName)> pools = {
                &componentPool.GetDenseEntities()...
            };

            auto minDenseIter = std::min_element(pools.begin(), pools.end(),
                [](const auto* a, const auto* b){ return a->size() < b->size(); }
            );
            mDenseEntities = *minDenseIter;
        }

        struct Iterator {
            
            using Category = std::forward_iterator_tag;
            using ValueType = std::tuple<uint32_t, ComponentName&...>;
            using Reference = ValueType;
            using Pointer = void;

            const ComponentView* componentView;
            size_t index;

            Iterator(const ComponentView* view, size_t i, bool advance = true) : componentView(view), index(i) {
                if(advance) {
                    AdvanceToValid();
                }
            }

            void AdvanceToValid() {
                auto& entities = *componentView->mDenseEntities;

                while(index < entities.size()) {
                    uint32_t entityID = entities[index];
                    // Checks if entity has every component listed
                    bool rightEntity = std::apply
                        ([&](auto&... componentPool){
                            return (... && componentPool.HasComponent(entityID));
                        }, 
                        componentView->mComponentPools
                    );
                    if(rightEntity) {
                        break;
                    }
                    ++index;
                }
            }

            Iterator& operator++() {
                ++index;
                AdvanceToValid();
                return *this;
            }

            Iterator operator++(int) {
                Iterator temp = *this;
                ++(*this);
                return temp;
            }

            bool operator==(Iterator const& other) const {
                return index == other.index && componentView == other.componentView;
            }

            bool operator!=(const Iterator& other) const {
                return !(*this == other);
            }

            Reference operator*() const {
                uint32_t entity = (*componentView->mDenseEntities)[index];

                return std::apply(
                    [&](auto&... componentPool){
                        return std::tuple<size_t, ComponentName&...> {
                            entity, componentPool.GetComponent(entity)...
                        };
                    },
                    componentView->mComponentPools
                );
            }
        };

        Iterator begin() const {
            if (mDenseEntities->empty()) { // Early exit if empty
                return end();
            }
            
            return {this, 0, true};
        }

        Iterator end() const {
            return {this, mDenseEntities->size(), false};
        }

        const size_t SizeHint() {
            return mDenseEntities->size();
        }

        const std::vector<uint32_t>* GetSmallestDense() {
            return mDenseEntities;
        }

    private:
        std::tuple<ComponentPool<ComponentName>&...> mComponentPools;
        const std::vector<uint32_t>* mDenseEntities; // Smallest array of entities
};

}