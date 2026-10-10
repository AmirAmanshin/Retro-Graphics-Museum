#pragma once
#include <glad/glad.h>
#include <vector>

class MazeMesh 
{
public:
    MazeMesh();
    ~MazeMesh();

    // Generate VAO VBO EBO from maze grid data
    void build(const std::vector<std::vector<int>>& grid);
    
    // Draw the geometry
    void draw() const;

private:
    GLuint VAO, VBO, EBO;
    unsigned int indexCount;
};