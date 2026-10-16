#include "Engine/MazeMesh.hpp"
#include <cmath>

MazeMesh::MazeMesh() : VAO(0), VBO(0), EBO(0), indexCount(0), solidIndexCount(0), solidOffset(0) {
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
}

MazeMesh::~MazeMesh() {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
}

// --- УРОВЕНЬ 1: Каркас и заливка ---
void MazeMesh::build(const std::vector<std::vector<int>>& grid) {
    std::vector<float> vertices;
    std::vector<unsigned int> lineIndices;
    std::vector<unsigned int> triIndices;

    if (grid.empty()) return;
    int height = grid.size();
    int width = grid[0].size();
    float blockSize = 1.0f;

    for (int z = 0; z < height; ++z) {
        for (int x = 0; x < width; ++x) {
            if (grid[z][x] == 1 || grid[z][x] == 2) { 
                unsigned int baseIdx = vertices.size() / 3;

                // Если это выход (2) - делаем сильный отступ, чтобы получить маленький парящий куб
                float m = (grid[z][x] == 2) ? 0.35f : 0.0f; 

                float x0 = x * blockSize + m;
                float x1 = (x + 1) * blockSize - m;
                float y0 = m;          
                float y1 = blockSize - m;     
                float z0 = z * blockSize + m;
                float z1 = (z + 1) * blockSize - m;

                // 8 вершин куба
                float cubeVertices[] = {
                    x0, y0, z1,  x1, y0, z1,  x1, y1, z1,  x0, y1, z1, // Перед
                    x0, y0, z0,  x1, y0, z0,  x1, y1, z0,  x0, y1, z0  // Зад
                };
                vertices.insert(vertices.end(), cubeVertices, cubeVertices + 24);

                // 1. Индексы для линий (24 индекса)
                unsigned int lIndices[] = {
                    0, 1,  1, 2,  2, 3,  3, 0,
                    4, 5,  5, 6,  6, 7,  7, 4,
                    0, 4,  1, 5,  2, 6,  3, 7
                };
                for (unsigned int i = 0; i < 24; ++i) lineIndices.push_back(lIndices[i] + baseIdx);

                // 2. Индексы для сплошных граней-треугольников (36 индексов)
                unsigned int tIndices[] = {
                    0, 1, 2, 2, 3, 0, // Перед
                    4, 5, 6, 6, 7, 4, // Зад
                    4, 0, 3, 3, 7, 4, // Лево
                    1, 5, 6, 6, 2, 1, // Право
                    3, 2, 6, 6, 7, 3, // Верх
                    4, 5, 1, 1, 0, 4  // Низ
                };
                for (unsigned int i = 0; i < 36; ++i) triIndices.push_back(tIndices[i] + baseIdx);
            }
        }
    }

    indexCount = lineIndices.size();
    solidIndexCount = triIndices.size();
    solidOffset = indexCount * sizeof(unsigned int);

    // Сливаем линии и треугольники в один общий буфер EBO
    std::vector<unsigned int> allIndices = lineIndices;
    allIndices.insert(allIndices.end(), triIndices.begin(), triIndices.end());

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, allIndices.size() * sizeof(unsigned int), allIndices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
}

void MazeMesh::draw() const {
    glBindVertexArray(VAO);
    // Рисуем линии с начала буфера
    glDrawElements(GL_LINES, indexCount, GL_UNSIGNED_INT, 0); 
    glBindVertexArray(0);
}

void MazeMesh::drawSolid() const {
    glBindVertexArray(VAO);
    // Рисуем треугольники со смещением (чтобы получить сплошные грани)
    glDrawElements(GL_TRIANGLES, solidIndexCount, GL_UNSIGNED_INT, (void*)(size_t)solidOffset); 
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
                float t = static_cast<float>(texType);

                float faceVertices[] = {
                    // Передняя грань (z1)
                    x0, y0, z1,  0.0f, 0.0f, t,
                    x1, y0, z1,  1.0f, 0.0f, t,
                    x1, y1, z1,  1.0f, 1.0f, t,
                    x0, y1, z1,  0.0f, 1.0f, t,
                    // Задняя грань (z0)
                    x1, y0, z0,  0.0f, 0.0f, t,
                    x0, y0, z0,  1.0f, 0.0f, t,
                    x0, y1, z0,  1.0f, 1.0f, t,
                    x1, y1, z0,  0.0f, 1.0f, t,
                    // Левая грань (x0)
                    x0, y0, z0,  0.0f, 0.0f, t,
                    x0, y0, z1,  1.0f, 0.0f, t,
                    x0, y1, z1,  1.0f, 1.0f, t,
                    x0, y1, z0,  0.0f, 1.0f, t,
                    // Правая грань (x1)
                    x1, y0, z1,  0.0f, 0.0f, t,
                    x1, y0, z0,  1.0f, 0.0f, t,
                    x1, y1, z0,  1.0f, 1.0f, t,
                    x1, y1, z1,  0.0f, 1.0f, t,
                    // Верхняя грань (y1)
                    x0, y1, z1,  0.0f, 0.0f, t,
                    x1, y1, z1,  1.0f, 0.0f, t,
                    x1, y1, z0,  1.0f, 1.0f, t,
                    x0, y1, z0,  0.0f, 1.0f, t,
                    // Нижняя грань (y0)
                    x0, y0, z0,  0.0f, 0.0f, t,
                    x1, y0, z0,  1.0f, 0.0f, t,
                    x1, y0, z1,  1.0f, 1.0f, t,
                    x0, y0, z1,  0.0f, 1.0f, t
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

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(5 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);
}

void MazeMesh::drawTextured(const std::vector<GLuint>& textures) const {
    glBindVertexArray(VAO);
    
    // 1. Сначала распределяем все текстуры по своим слотам (GL_TEXTURE0, GL_TEXTURE1 и т.д.)
    for (int i = 0; i < 4; ++i) {
        if (i < static_cast<int>(textures.size()) && textures[i] != 0) {
            glActiveTexture(GL_TEXTURE0 + i);
            glBindTexture(GL_TEXTURE_2D, textures[i]);
        }
    }

    // 2. Теперь отрисовываем батчи. Шейдер сам выберет нужный слот благодаря TexIndex.
    for (int i = 0; i < 4; ++i) {
        if (batches[i].count > 0) {
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