#pragma once
#include <vector>
#include <glad/glad.h>

struct LitMesh {
    GLuint VAO = 0;
    GLuint VBO = 0;
    int vertexCount = 0;

    void draw() const;
    void destroy();
};

class PrimitiveMesh {
public:
    static LitMesh CreateBox(float width, float height, float depth);
    static void BuildLitMaze(const std::vector<std::vector<int>>& grid, LitMesh outWallBatches[4]);
};