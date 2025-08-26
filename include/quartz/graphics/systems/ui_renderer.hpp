#pragma once
// Third_party libraries
#include "glad/glad.h"

// Own libraries
#include "./bismuth/registry.hpp"
#include "quartz/graphics/shader.hpp"
#include "quartz/ui/components/gui_mesh.hpp"
#include "quartz/ui/components/text_mesh.hpp"
#include "quartz/core/components/gui_camera_component.hpp"

namespace quartz {
    
class UiRendererSystem {
    public:
        UiRendererSystem();

        void Update(bismuth::Registry& registry);

    private:
        GLuint mGuiProgram;
        GLuint mTextProgram;
        
        GLuint mUniformProjectionMatrix;
        GLuint mUniformProjectionMatrix2;
        GLuint mUniformFontAtlas;
};

}