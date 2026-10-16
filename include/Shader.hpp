#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

class Shader {
public:
    GLuint ID;

    Shader(const char* vertexPath, const char* fragmentPath);
    ~Shader();

    void use() const;
    void setMat4(const std::string &name, const glm::mat4 &mat) const;
    void setVec4(const std::string &name, float x, float y, float z, float w) const;

private:
    void checkCompileErrors(GLuint shader, std::string type);
};