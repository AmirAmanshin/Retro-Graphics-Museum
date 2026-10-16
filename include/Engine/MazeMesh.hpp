#pragma once
#include <glad/glad.h>
#include <vector>
#include <cstdint>

struct BatchInfo {
    unsigned int offset = 0;
    unsigned int count = 0;
};

class MazeMesh {
public:
    MazeMesh();
    ~MazeMesh();

    // Для Уровня 1
    void build(const std::vector<std::vector<int>>& grid);
    void draw() const;       // Рисует зеленые ребра
    void drawSolid() const;  // Рисует черные грани для скрытия заднего фона

    // Для Уровня 2
    void buildTextured(const std::vector<std::vector<int>>& grid);
    void drawTextured(const std::vector<GLuint>& textures) const;

private:
    GLuint VAO, VBO, EBO;
    
    // Переменные для Уровня 1
    unsigned int indexCount;       // Индексы для линий (каркас)
    unsigned int solidIndexCount;  // Индексы для полигонов (заливка)
    unsigned int solidOffset;      // Смещение в буфере для полигонов

    // Переменные для Уровня 2
    BatchInfo batches[4];
};