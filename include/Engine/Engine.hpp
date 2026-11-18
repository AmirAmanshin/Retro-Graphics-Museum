#pragma once
#include <string>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "Levels/Level.hpp"
#include "Engine/HudOverlay.hpp"

class Engine {
public:
    Engine(unsigned int width, unsigned int height, const char* title);
    ~Engine();

    bool init();
    void run();
    void changeLevel(Level* newLevel);

private:
    unsigned int width, height;
    const char* title;
    GLFWwindow* window;
    Level* currentLevel;
    HudOverlay hud;
    int currentLevelIndex;
    std::string currentLevelName;
    bool hKeyPressed;

    static void framebuffer_size_callback(GLFWwindow* window, int width, int height);
};