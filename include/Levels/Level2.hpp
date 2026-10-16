#pragma once
#include <vector>
#include "Level.hpp"
#include "Engine/Engine.hpp"
#include "Engine/MazeGenerator.hpp"
#include "Engine/MazeMesh.hpp"
#include "Engine/Shader.hpp"
#include "Engine/Camera.hpp"

class Level2 : public Level {
public:
    Level2(Engine* engine);
    ~Level2();

    void init() override;
    void processInput(GLFWwindow* window, float deltaTime) override;
    void update(float deltaTime) override;
    void render() override;

private:
    Engine* gameEngine;
    Camera camera;
    Shader* shader;
    MazeGenerator* maze;
    MazeMesh* mazeMesh;

    std::vector<GLuint> textures; // Массив всех загруженных текстур

    bool firstMouse;
    float lastX, lastY;

    bool noclip;
    bool fKeyPressed;

    bool checkCollision(float x, float z);
    GLuint loadTexture(const char* path);
};