#pragma once
// C++ standard libraries
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


    private:
        std::vector<IComponentPool> mComponentPool;
};