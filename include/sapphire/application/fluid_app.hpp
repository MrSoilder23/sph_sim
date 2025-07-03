#pragma once
// C++ standard libraries
#include <string>

// Third_party libraries
#include <glm/glm.hpp>

// Own libraries
#include "quartz/engine.hpp"
#include "bismuth/registry.hpp"
#include "sapphire/utility/window_data.hpp"
#include "sapphire/utility/data_buffers.hpp"

#include "quartz/core/systems/camera_system.hpp"
#include "quartz/ui/systems/gui_camera_system.hpp"
#include "quartz/ui/systems/gui_vertex_setup.hpp"
#include "quartz/ui/systems/style_system.hpp"
#include "quartz/graphics/systems/ui_renderer.hpp"
// #include "quartz/graphics/instanced_renderer_system.hpp"
#include "sapphire/systems/gpu_sphere_data_system.hpp"
#include "sapphire/systems/particle_system.hpp"

#include "quartz/ui/components/gui_mesh.hpp"
#include "quartz/ui/components/gui_object_component.hpp"
#include "quartz/core/components/gui_camera_component.hpp"
#include "quartz/core/components/instance_component.hpp"
#include "quartz/core/components/sphere_component.hpp"
#include "sapphire/components/density_component.hpp"
#include "sapphire/components/force_component.hpp"
#include "sapphire/components/mass_component.hpp"
#include "sapphire/components/pressure_component.hpp"
#include "sapphire/components/velocity_component.hpp"
#include "sapphire/components/energy_component.hpp"

class FluidApp {
    public:
        FluidApp(std::string configFilePath);
        ~FluidApp();
        
        void Run();

    private:
        void Loop(float deltaTime);
        void System(float deltaTime);
        void Event(float deltaTime);

        void SpawnParticles(int mouseX, int mouseY);

        // Main helpers
        void FpsCounter(float deltaTime);
        void InitEntities();
        
    private:
        std::string mConfigFilePath = "./config/config.json";

        quartz::Engine mEngine;
        bismuth::Registry mRegistry;

        WindowData mWindowData;

        DataBuffers mDataBuffers;
        ParticleSystem mParticleSystem;

};