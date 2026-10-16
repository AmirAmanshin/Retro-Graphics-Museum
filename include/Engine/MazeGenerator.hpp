#pragma once
#include <vector>

class MazeGenerator
{
public:
    MazeGenerator(int width, int height);
    
    void generate();
    
    void printToConsole() const;
    
    const std::vector<std::vector<int>>& getGrid() const;

    int getExitX() const { return exitX; }
    int getExitY() const { return exitY; }

private:
    int width, height;
    std::vector<std::vector<int>> grid;

    void carvePassagesFrom(int cx, int cy);

    int exitX, exitY;
};