#include "Engine/PrimitiveMesh.hpp"
#include <random>

void LitMesh::draw() const {
    if (VAO == 0 || vertexCount == 0) return;
    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, vertexCount);
    glBindVertexArray(0);
}

void LitMesh::destroy() {
    if (VAO != 0) glDeleteVertexArrays(1, &VAO);
    if (VBO != 0) glDeleteBuffers(1, &VBO);
    VAO = 0;
    VBO = 0;
    vertexCount = 0;
}

static LitMesh UploadLitVertices(const std::vector<float>& data) {
    LitMesh mesh;
    mesh.vertexCount = static_cast<int>(data.size() / 8);
    if (mesh.vertexCount == 0) return mesh;

    glGenVertexArrays(1, &mesh.VAO);
    glGenBuffers(1, &mesh.VBO);

    glBindVertexArray(mesh.VAO);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.VBO);
    glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(float), data.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(5 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);
    return mesh;
}

LitMesh PrimitiveMesh::CreateBox(float width, float height, float depth) {
    float hx = width * 0.5f;
    float hy = height * 0.5f;
    float hz = depth * 0.5f;

    std::vector<float> v = {
        // Front (+Z)
        -hx, -hy,  hz,  0.0f, 0.0f,  0.0f,  0.0f,  1.0f,
         hx, -hy,  hz,  1.0f, 0.0f,  0.0f,  0.0f,  1.0f,
         hx,  hy,  hz,  1.0f, 1.0f,  0.0f,  0.0f,  1.0f,
        -hx, -hy,  hz,  0.0f, 0.0f,  0.0f,  0.0f,  1.0f,
         hx,  hy,  hz,  1.0f, 1.0f,  0.0f,  0.0f,  1.0f,
        -hx,  hy,  hz,  0.0f, 1.0f,  0.0f,  0.0f,  1.0f,
        // Back (-Z)
         hx, -hy, -hz,  0.0f, 0.0f,  0.0f,  0.0f, -1.0f,
        -hx, -hy, -hz,  1.0f, 0.0f,  0.0f,  0.0f, -1.0f,
        -hx,  hy, -hz,  1.0f, 1.0f,  0.0f,  0.0f, -1.0f,
         hx, -hy, -hz,  0.0f, 0.0f,  0.0f,  0.0f, -1.0f,
        -hx,  hy, -hz,  1.0f, 1.0f,  0.0f,  0.0f, -1.0f,
         hx,  hy, -hz,  0.0f, 1.0f,  0.0f,  0.0f, -1.0f,
        // Left (-X)
        -hx, -hy, -hz,  0.0f, 0.0f, -1.0f,  0.0f,  0.0f,
        -hx, -hy,  hz,  1.0f, 0.0f, -1.0f,  0.0f,  0.0f,
        -hx,  hy,  hz,  1.0f, 1.0f, -1.0f,  0.0f,  0.0f,
        -hx, -hy, -hz,  0.0f, 0.0f, -1.0f,  0.0f,  0.0f,
        -hx,  hy,  hz,  1.0f, 1.0f, -1.0f,  0.0f,  0.0f,
        -hx,  hy, -hz,  0.0f, 1.0f, -1.0f,  0.0f,  0.0f,
        // Right (+X)
         hx, -hy,  hz,  0.0f, 0.0f,  1.0f,  0.0f,  0.0f,
         hx, -hy, -hz,  1.0f, 0.0f,  1.0f,  0.0f,  0.0f,
         hx,  hy, -hz,  1.0f, 1.0f,  1.0f,  0.0f,  0.0f,
         hx, -hy,  hz,  0.0f, 0.0f,  1.0f,  0.0f,  0.0f,
         hx,  hy, -hz,  1.0f, 1.0f,  1.0f,  0.0f,  0.0f,
         hx,  hy,  hz,  0.0f, 1.0f,  1.0f,  0.0f,  0.0f,
        // Top (+Y)
        -hx,  hy,  hz,  0.0f, 0.0f,  0.0f,  1.0f,  0.0f,
         hx,  hy,  hz,  1.0f, 0.0f,  0.0f,  1.0f,  0.0f,
         hx,  hy, -hz,  1.0f, 1.0f,  0.0f,  1.0f,  0.0f,
        -hx,  hy,  hz,  0.0f, 0.0f,  0.0f,  1.0f,  0.0f,
         hx,  hy, -hz,  1.0f, 1.0f,  0.0f,  1.0f,  0.0f,
        -hx,  hy, -hz,  0.0f, 1.0f,  0.0f,  1.0f,  0.0f,
        // Bottom (-Y)
        -hx, -hy, -hz,  0.0f, 0.0f,  0.0f, -1.0f,  0.0f,
         hx, -hy, -hz,  1.0f, 0.0f,  0.0f, -1.0f,  0.0f,
         hx, -hy,  hz,  1.0f, 1.0f,  0.0f, -1.0f,  0.0f,
        -hx, -hy, -hz,  0.0f, 0.0f,  0.0f, -1.0f,  0.0f,
         hx, -hy,  hz,  1.0f, 1.0f,  0.0f, -1.0f,  0.0f,
        -hx, -hy,  hz,  0.0f, 1.0f,  0.0f, -1.0f,  0.0f
    };

    return UploadLitVertices(v);
}

void PrimitiveMesh::BuildLitMaze(const std::vector<std::vector<int>>& grid, LitMesh outWallBatches[4]) {
    std::vector<float> batches[4];
    std::mt19937 gen(42);
    std::uniform_int_distribution<int> rareDist(0, 99);

    auto pushQuad = [](std::vector<float>& dst,
                       float x0, float y0, float z0,
                       float x1, float y1, float z1,
                       float x2, float y2, float z2,
                       float x3, float y3, float z3,
                       float nx, float ny, float nz) {
        float quad[] = {
            x0, y0, z0,  0.0f, 0.0f,  nx, ny, nz,
            x1, y1, z1,  1.0f, 0.0f,  nx, ny, nz,
            x2, y2, z2,  1.0f, 1.0f,  nx, ny, nz,
            x0, y0, z0,  0.0f, 0.0f,  nx, ny, nz,
            x2, y2, z2,  1.0f, 1.0f,  nx, ny, nz,
            x3, y3, z3,  0.0f, 1.0f,  nx, ny, nz
        };
        dst.insert(dst.end(), std::begin(quad), std::end(quad));
    };

    int rows = static_cast<int>(grid.size());
    int cols = static_cast<int>(grid[0].size());

    for (int z = 0; z < rows; ++z) {
        for (int x = 0; x < cols; ++x) {
            int cell = grid[z][x];
            if (cell == 0) continue;

            // cell == 2 -> дверь выхода (слот 3); иначе 88% основная стена (0), 12% редкая (2)
            int texSlot = (cell == 2) ? 3 : ((rareDist(gen) < 12) ? 2 : 0);
            float x0 = static_cast<float>(x);
            float z0 = static_cast<float>(z);
            float x1 = x0 + 1.0f;
            float z1 = z0 + 1.0f;

            if (z + 1 >= rows || grid[z + 1][x] == 0)
                pushQuad(batches[texSlot], x0, 0.0f, z1, x1, 0.0f, z1, x1, 1.0f, z1, x0, 1.0f, z1, 0.0f, 0.0f, 1.0f);
            if (z - 1 < 0 || grid[z - 1][x] == 0)
                pushQuad(batches[texSlot], x1, 0.0f, z0, x0, 0.0f, z0, x0, 1.0f, z0, x1, 1.0f, z0, 0.0f, 0.0f, -1.0f);
            if (x - 1 < 0 || grid[z][x - 1] == 0)
                pushQuad(batches[texSlot], x0, 0.0f, z0, x0, 0.0f, z1, x0, 1.0f, z1, x0, 1.0f, z0, -1.0f, 0.0f, 0.0f);
            if (x + 1 >= cols || grid[z][x + 1] == 0)
                pushQuad(batches[texSlot], x1, 0.0f, z1, x1, 0.0f, z0, x1, 1.0f, z0, x1, 1.0f, z1, 1.0f, 0.0f, 0.0f);
        }
    }

    for (int i = 0; i < 4; ++i) {
        outWallBatches[i].destroy();
        outWallBatches[i] = UploadLitVertices(batches[i]);
    }
}