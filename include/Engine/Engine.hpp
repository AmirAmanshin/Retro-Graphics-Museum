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
    void requestQuit();

    HudOverlay& getHud() { return hud; }
    void toggleCanonicalFps() { useCanonicalFps = !useCanonicalFps; }
    bool isCanonicalFps() const { return useCanonicalFps; }
    float getActiveTargetFps() const;

    float standardFps        = 144.0f;

    float level1CanonicalFps = 10.0f;
    float level2CanonicalFps = 15.0f;
    float level3CanonicalFps = 15.0f;
    float level4CanonicalFps = 20.0f;
    float menuCanonicalFps   = 60.0f;

private:
    unsigned int width, height;
    const char* title;
    GLFWwindow* window;
    Level* currentLevel;
    Level* pendingLevel;
    HudOverlay hud;
    int currentLevelIndex;
    std::string currentLevelName;
    bool hKeyPressed;

    bool useCanonicalFps;
    bool cKeyPressed;

    void applyLevelChange(Level* newLevel);
    static void framebuffer_size_callback(GLFWwindow* window, int width, int height);
};