#include "./quartz/graphics/shader.hpp"
#include <vector>
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
}

GLuint shader::CompileShader(GLuint type, const std::string& shaderPath) {
    GLuint shaderObject;

    shaderObject = glCreateShader(type);

    std::string shaderCode = LoadShaderAsString(shaderPath);
    
    const char* src = shaderCode.c_str();
    glShaderSource(shaderObject, 1, &src, nullptr);
    glCompileShader(shaderObject);

    GLint success;
    glGetShaderiv(shaderObject, GL_COMPILE_STATUS, &success);
    if (!success) {
        GLint length;
        glGetShaderiv(shaderObject, GL_INFO_LOG_LENGTH, &length);
        std::vector<GLchar> log(length);
        glGetShaderInfoLog(shaderObject, length, &length, log.data());
        
        std::cerr << "SHADER COMPILE ERROR (" << shaderPath << "):\n"
                  << log.data() << std::endl;
        return 0; // Return 0 to indicate failure
    }

    return shaderObject;

}

GLuint shader::LinkProgram(GLuint& shader) {
    GLuint programObject = glCreateProgram();

    glAttachShader(programObject, shader);
    glLinkProgram(programObject);

    
    GLint success;
    glGetProgramiv(programObject, GL_LINK_STATUS, &success);
    if (!success) {
        GLint length;
        glGetProgramiv(programObject, GL_INFO_LOG_LENGTH, &length);
        std::vector<GLchar> log(length);
        glGetProgramInfoLog(programObject, length, &length, log.data());
        
        std::cerr << "PROGRAM LINK ERROR:\n"
                  << log.data() << std::endl;
        return 0; // Return 0 to indicate failure
    }


    return programObject;
}

GLuint shader::CreateGraphicsPipeline(const std::string& vertexShaderPath, const std::string& fragmentShaderPath) {
    GLuint programObject = glCreateProgram();

    GLuint myVertexShader =   shader::CompileShader(GL_VERTEX_SHADER, vertexShaderPath);
    GLuint myFragmentShader = shader::CompileShader(GL_FRAGMENT_SHADER, fragmentShaderPath);

    glAttachShader(programObject, myVertexShader);
    glAttachShader(programObject, myFragmentShader);
    glLinkProgram(programObject);

    glValidateProgram(programObject);

    return programObject;
}

int shader::FindUniformLocation(GLuint pipeline, const GLchar* name) {
    GLint uniformLocation = glGetUniformLocation(pipeline,name);
    if(uniformLocation < 0) {
        std::cerr << "Could not find location of: " << name << std::endl;
        exit(1); // Change to exit failure
    }

    return uniformLocation;
}