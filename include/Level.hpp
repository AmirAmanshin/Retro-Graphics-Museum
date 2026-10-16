#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>

class Engine;

class Level {
public:
    virtual ~Level() = default;
    virtual void init() = 0;
    virtual void processInput(GLFWwindow* window, float deltaTime) = 0;
    virtual void update(float deltaTime) = 0;
    virtual void render() = 0;
};