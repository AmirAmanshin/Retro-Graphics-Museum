#pragma once
#include <glad/glad.h>
#include <vector>

struct BatchInfo {
    unsigned int offset = 0;
    unsigned int count = 0;
};

class MazeMesh {
public:
    MazeMesh();
    ~MazeMesh();

    // Для Уровня 1 (линии / каркас)
    void build(const std::vector<std::vector<int>>& grid);
    void draw() const;

    // Для Уровня 2 (мультитекстурированные стены)
    void buildTextured(const std::vector<std::vector<int>>& grid);
    void drawTextured(const std::vector<GLuint>& textures) const;

private:
    GLuint VAO, VBO, EBO;
    unsigned int indexCount; // Использовать для build / draw (GL_LINES)
    BatchInfo batches[4];    // Использовать для buildTextured / drawTextured (GL_TRIANGLES)
};