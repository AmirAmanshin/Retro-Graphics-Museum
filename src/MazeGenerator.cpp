#include "MazeGenerator.hpp"
#include <iostream>
#include <algorithm>
#include <random>
#include <chrono>

MazeGenerator::MazeGenerator(int w, int h) : width(w), height(h), exitX(0), exitY(0)
{
    // Initialize the grid with only walls
    grid.assign(height, std::vector<int>(width, 1));
}

void MazeGenerator::generate()
{
    // Check if the dimensions are odd, if not, abort the generation
    if (width % 2 == 0 || height % 2 == 0)
    {
        std::cerr << "Width and height must be odd numbers." << std::endl;
        return;
    }
    
    // Carving starts from cell (1, 1)
    carvePassagesFrom(1, 1);

    // Set the exit point
    for (int x = width - 2; x > 0; --x)
    {
        if (grid[height - 2][x] == 0)
        {
            exitX = x;
            exitY = height - 1; // Set exit at the bottom row
            grid[exitY][exitX] = 2; // (2) means exit point
            break;
        }
    }
}

void MazeGenerator::carvePassagesFrom(int cx, int cy)
{
    // Carves space out of current cell
    grid[cy][cx] = 0;

    // Possible directions to move: up, right, down, left
    // Move by 2 to ensure we skip over walls
    int dx[] = { 0, 2, 0, -2 };
    int dy[] = { -2, 0, 2, 0 };
    int directions[] = { 0, 1, 2, 3 };

    // Shuffle directions for random generation
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::shuffle(directions, directions + 4, std::default_random_engine(seed));

    for (int i = 0; i < 4; ++i)
    {
        int dir = directions[i];
        int nx = cx + dx[dir];
        int ny = cy + dy[dir];

        // Check if the new position is within bounds and is a wall
        if (ny > 0 && ny < height - 1 && nx > 0 && nx < width - 1)
        {
            if (grid[ny][nx] == 1){
                // Carve a passage between the current cell and the new cell
                grid[cy + dy[dir] / 2][cx + dx[dir] / 2] = 0;
                // Recursively carve passages from the new cell
                carvePassagesFrom(nx, ny);
            }
        }
    }
}

void MazeGenerator::printToConsole() const 
{
    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            // Print walls as "##", paths as "  ", and exit as "$$"
            if (grid[y][x] == 1) std::cout << "##";
            else if (grid[y][x] == 2) std::cout << "$$";
            else std::cout << "  ";
        }
        std::cout << "\n";
    }
}

const std::vector<std::vector<int>>& MazeGenerator::getGrid() const
{
    return grid;
}