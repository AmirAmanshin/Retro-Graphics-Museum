#include "Engine/MazeMesh.hpp"
#include <cmath>

MazeMesh::MazeMesh() : VAO(0), VBO(0), EBO(0), indexCount(0) {
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
}

MazeMesh::~MazeMesh() {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
}

// --- УРОВЕНЬ 1: Каркасные линии ---
void MazeMesh::build(const std::vector<std::vector<int>>& grid) {
    std::vector<float> vertices;
    std::vector<unsigned int> indices;

    if (grid.empty()) return;
    int height = grid.size();
    int width = grid[0].size();
    float blockSize = 1.0f;

    for (int z = 0; z < height; ++z) {
        for (int x = 0; x < width; ++x) {
            if (grid[z][x] == 1 || grid[z][x] == 2) { 
                unsigned int baseIdx = vertices.size() / 3;

                float x0 = x * blockSize;
                float x1 = x0 + blockSize;
                float y0 = 0.0f;          
                float y1 = blockSize;     
                float z0 = z * blockSize;
                float z1 = z0 + blockSize;

                float cubeVertices[] = {
                    x0, y0, z1,  x1, y0, z1,  x1, y1, z1,  x0, y1, z1, // Перед
                    x0, y0, z0,  x1, y0, z0,  x1, y1, z0,  x0, y1, z0  // Зад
                };
                vertices.insert(vertices.end(), cubeVertices, cubeVertices + 24);

                unsigned int lineIndices[] = {
                    0, 1,  1, 2,  2, 3,  3, 0,
                    4, 5,  5, 6,  6, 7,  7, 4,
                    0, 4,  1, 5,  2, 6,  3, 7
                };

                for (unsigned int i = 0; i < 24; ++i) {
                    indices.push_back(lineIndices[i] + baseIdx);
                }
            }
        }
    }

    indexCount = indices.size();

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
}

void MazeMesh::draw() const {
    glBindVertexArray(VAO);
    glDrawElements(GL_LINES, indexCount, GL_UNSIGNED_INT, 0); 
    glBindVertexArray(0);
}

// --- УРОВЕНЬ 2: Текстурированные треугольники ---
void MazeMesh::buildTextured(const std::vector<std::vector<int>>& grid) {
    std::vector<float> vertices;
    std::vector<unsigned int> batchIndices[4];

    if (grid.empty()) return;
    int height = grid.size();
    int width = grid[0].size();
    float blockSize = 1.0f;

    for (int z = 0; z < height; ++z) {
        for (int x = 0; x < width; ++x) {
            int cell = grid[z][x];
            
            if (cell == 1 || cell == 2) {
                int texType = 0;
                
                if (cell == 2) {
                    texType = 3; // exit.png
                } else {
                    int rnd = std::abs(x * 73856093 ^ z * 19349663) % 100;
                    if (rnd < 70) texType = 0;      // wall1.png
                    else if (rnd < 85) texType = 1; // wall2.png
                    else texType = 2;               // wall3.png
                }

                unsigned int baseIdx = vertices.size() / 5;

                float x0 = x * blockSize;
                float x1 = x0 + blockSize;
                float y0 = 0.0f;          
                float y1 = blockSize;     
                float z0 = z * blockSize;
                float z1 = z0 + blockSize;

                float faceVertices[] = {
                    // Передняя грань (z1)
                    x0, y0, z1,  0.0f, 0.0f,
                    x1, y0, z1,  1.0f, 0.0f,
                    x1, y1, z1,  1.0f, 1.0f,
                    x0, y1, z1,  0.0f, 1.0f,
                    // Задняя грань (z0)
                    x1, y0, z0,  0.0f, 0.0f,
                    x0, y0, z0,  1.0f, 0.0f,
                    x0, y1, z0,  1.0f, 1.0f,
                    x1, y1, z0,  0.0f, 1.0f,
                    // Левая грань (x0)
                    x0, y0, z0,  0.0f, 0.0f,
                    x0, y0, z1,  1.0f, 0.0f,
                    x0, y1, z1,  1.0f, 1.0f,
                    x0, y1, z0,  0.0f, 1.0f,
                    // Правая грань (x1)
                    x1, y0, z1,  0.0f, 0.0f,
                    x1, y0, z0,  1.0f, 0.0f,
                    x1, y1, z0,  1.0f, 1.0f,
                    x1, y1, z1,  0.0f, 1.0f,
                    // Верхняя грань (y1)
                    x0, y1, z1,  0.0f, 0.0f,
                    x1, y1, z1,  1.0f, 0.0f,
                    x1, y1, z0,  1.0f, 1.0f,
                    x0, y1, z0,  0.0f, 1.0f,
                    // Нижняя грань (y0)
                    x0, y0, z0,  0.0f, 0.0f,
                    x1, y0, z0,  1.0f, 0.0f,
                    x1, y0, z1,  1.0f, 1.0f,
                    x0, y0, z1,  0.0f, 1.0f
                };
                vertices.insert(vertices.end(), faceVertices, faceVertices + 120);

                unsigned int faceIndices[] = {
                    0, 1, 2, 2, 3, 0,       // Перед
                    4, 5, 6, 6, 7, 4,       // Зад
                    8, 9, 10, 10, 11, 8,    // Лево
                    12, 13, 14, 14, 15, 12, // Право
                    16, 17, 18, 18, 19, 16, // Верх
                    20, 21, 22, 22, 23, 20  // Низ
                };

                for (unsigned int i = 0; i < 36; ++i) {
                    batchIndices[texType].push_back(faceIndices[i] + baseIdx);
                }
            }
        }
    }

    std::vector<unsigned int> allIndices;
    unsigned int currentOffset = 0;

    for (int b = 0; b < 4; ++b) {
        batches[b].offset = currentOffset;
        batches[b].count = batchIndices[b].size();
        allIndices.insert(allIndices.end(), batchIndices[b].begin(), batchIndices[b].end());
        currentOffset += batchIndices[b].size();
    }

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, allIndices.size() * sizeof(unsigned int), allIndices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
}

void MazeMesh::drawTextured(const std::vector<GLuint>& textures) const {
    glBindVertexArray(VAO);
    for (int i = 0; i < 4; ++i) {
        if (batches[i].count > 0 && i < static_cast<int>(textures.size())) {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, textures[i]);
            glDrawElements(
                GL_TRIANGLES,
                batches[i].count,
                GL_UNSIGNED_INT,
                (void*)(batches[i].offset * sizeof(unsigned int))
            );
        }
    }
    glBindVertexArray(0);
}