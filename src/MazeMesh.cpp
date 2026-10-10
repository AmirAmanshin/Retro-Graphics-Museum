#include "MazeMesh.hpp"

MazeMesh::MazeMesh() : VAO(0), VBO(0), EBO(0), indexCount(0) 
{
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
}

MazeMesh::~MazeMesh() 
{
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
}

void MazeMesh::build(const std::vector<std::vector<int>>& grid) 
{
    std::vector<float> vertices;
    std::vector<unsigned int> indices;

    if (grid.empty()) return;
    int height = grid.size();
    int width = grid[0].size();

    // Size of one block
    float blockSize = 1.0f;

    for (int z = 0; z < height; ++z)
    {
        for (int x = 0; x < width; ++x)
        {
            // If the cell is a wall, create a cube
            if (grid[z][x] == 1)
            {
                // Base index for the current cube's vertices
                unsigned int baseIdx = vertices.size() / 3;

                // Calculate the coordinates of the cube's corners
                float x0 = x * blockSize;
                float x1 = x0 + blockSize;
                float y0 = 0.0f;          // Floor
                float y1 = blockSize;     // Ceiling
                float z0 = z * blockSize;
                float z1 = z0 + blockSize;

                // Add the 8 vertices of the cube (x, y, z)
                float cubeVertices[] =
                {
                    x0, y0, z1,  x1, y0, z1,  x1, y1, z1,  x0, y1, z1, // Front face
                    x0, y0, z0,  x1, y0, z0,  x1, y1, z0,  x0, y1, z0  // Back face
                };
                vertices.insert(vertices.end(), cubeVertices, cubeVertices + 24);

                // Indices for 12 triangles
                unsigned int cubeIndices[] =
                {
                    0, 1, 2, 2, 3, 0, // Front
                    1, 5, 6, 6, 2, 1, // Right
                    5, 4, 7, 7, 6, 5, // Back
                    4, 0, 3, 3, 7, 4, // Left
                    3, 2, 6, 6, 7, 3, // Top
                    4, 5, 1, 1, 0, 4  // Bottom
                };

                // Shift indices by baseIdx and add to the array
                for (unsigned int i = 0; i < 36; ++i)
                {
                    indices.push_back(cubeIndices[i] + baseIdx);
                }
            }
        }
    }

    indexCount = indices.size();

    // Load the data into video memory
    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    // Set vertex attribute pointers
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
}

void MazeMesh::draw() const 
{
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}