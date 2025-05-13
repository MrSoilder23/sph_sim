#include "./quartz/graphics/shader.hpp"

namespace {
    std::string LoadShaderAsString(const std::string& src) {
        std::string result = "";

        std::string line = "";
        std::ifstream myFile(src.c_str());

        if(myFile.is_open()) {
            while(std::getline(myFile, line)) {
                result += line + "\n";
            }
            myFile.close();
        }

        return result;
    }

    GLuint CompileShader(GLuint type, const std::string& source) {
        GLuint shaderObject;

        shaderObject = glCreateShader(type);

        const char* src = source.c_str();
        glShaderSource(shaderObject, 1, &src, nullptr);
        glCompileShader(shaderObject);

        return shaderObject;
        
    }

    GLuint CreateShaderProgram(const std::string& vertexShaderSource, const std::string& fragmentShaderSource) {
        GLuint programObject = glCreateProgram();

        GLuint myVertexShader = CompileShader(GL_VERTEX_SHADER, vertexShaderSource);
        GLuint myFragmentShader = CompileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);

        glAttachShader(programObject ,myVertexShader);
        glAttachShader(programObject ,myFragmentShader);
        glLinkProgram(programObject);

        glValidateProgram(programObject);

        return programObject;
    }

}

GLuint shader::CreateGraphicsPipeline(const std::string& _vertexShaderSource, const std::string& _fragmentShaderSource) {
    std::string vertexShaderSource = LoadShaderAsString(_vertexShaderSource);
    std::string fragmentShaderSource = LoadShaderAsString(_fragmentShaderSource);

    return CreateShaderProgram(vertexShaderSource, fragmentShaderSource);
}

int shader::FindUniformLocation(GLuint pipeline, const GLchar* name) {
    GLint uniformLocation = glGetUniformLocation(pipeline,name);
    if(uniformLocation < 0) {
        std::cerr << "Could not find location of: " << name << std::endl;
        exit(1); // Change to exit failure
    }

    return uniformLocation;
}