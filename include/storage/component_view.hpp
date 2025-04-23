#pragma once
// C++ standard libraries
#include <array>
#include <cstddef>
#include <iterator>
#include <tuple>
#include <vector>

// Own libraries
#include "component_pool.hpp"

template<typename... ComponentName>
class ComponentView {
    static_assert(sizeof...(ComponentName) > 0, "ComponentView requires at least one component type");

    public:

        ComponentView(ComponentPool<ComponentName>&... componentPool) : mComponentPools(componentPool...) {
            std::array<const std::vector<size_t>*, sizeof...(ComponentName)> array = {
                &componentPool.GetDenseEntities()...
            };

            mDenseEntities = array[0];
            for(auto ptr : array) {
                if(ptr->size() < mDenseEntities->size()) {
                    mDenseEntities = ptr;
                }
            }
        }

        struct Iterator {
            
            using Category = std::forward_iterator_tag;
            using ValueType = std::tuple<size_t, ComponentName&...>;
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
                    size_t entityID = entities[index];
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
                Iterator tmp = *this;
                ++(*this);
                return tmp;
            }

            bool operator==(Iterator const& other) const {
                return index == other.index && componentView == other.componentView;
            }

            bool operator!=(const Iterator& other) const {
                return !(*this == other);
            }

            Reference operator*() const {
                size_t entity = (*componentView->mDenseEntities)[index];

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
            return {this, 0, true};
        }

        Iterator end() const {
            return {this, mDenseEntities->size(), false};
        }

    private:
        std::tuple<ComponentPool<ComponentName>&...> mComponentPools;
        const std::vector<size_t>* mDenseEntities; // Smallest array of entities
};