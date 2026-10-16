#pragma once
#include <Levels/Level.hpp>
#include <Engine/Engine.hpp>
#include <Engine/MazeGenerator.hpp>
#include <Engine/MazeMesh.hpp>
#include <Engine/Shader.hpp>
#include <Engine/Camera.hpp>

class Level1 : public Level {
public:
    Level1(Engine* engine);
    ~Level1();

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

    glm::vec3 targetPos;
    float targetYaw;
    bool noclip;
    bool fKeyPressed;

    bool checkCollision(float x, float z);
};