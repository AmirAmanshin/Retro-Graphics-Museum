#include "Engine/Engine.hpp"
#include <iostream>
#include <cstdio>

#include "Levels/Level1.hpp"
#include "Levels/Level2.hpp"
#include "Levels/Level3.hpp"
#include "Levels/Level4.hpp"

Engine::Engine(unsigned int w, unsigned int h, const char* t)
    : width(w), height(h), title(t), window(nullptr), currentLevel(nullptr),
      currentLevelIndex(1), currentLevelName("RETRO GRAPHICS MUSEUM"), hKeyPressed(false) {}

Engine::~Engine() {
    if (currentLevel) delete currentLevel;
    glfwTerminate();
}

bool Engine::init() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(width, height, title, NULL, NULL);
    if (!window) return false;

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) return false;

    glEnable(GL_DEPTH_TEST);
    hud.init();
    return true;
}

void Engine::changeLevel(Level* newLevel) {
    if (currentLevel) delete currentLevel;
    currentLevel = newLevel;

    if (dynamic_cast<Level4*>(currentLevel)) {
        currentLevelIndex = 4;
        currentLevelName = "ERA 4: HORROR (BLINN-PHONG)";
    } else if (dynamic_cast<Level3*>(currentLevel)) {
        currentLevelIndex = 3;
        currentLevelName = "ERA 3: 3D + BILLBOARDS";
    } else if (dynamic_cast<Level2*>(currentLevel)) {
        currentLevelIndex = 2;
        currentLevelName = "ERA 2: 2.5D TEXTURED MAZE";
    } else if (dynamic_cast<Level1*>(currentLevel)) {
        currentLevelIndex = 1;
        currentLevelName = "ERA 1: 3D WIREFRAME";
    } else {
        currentLevelIndex = 0;
        currentLevelName = "RETRO GRAPHICS MUSEUM";
    }

    currentLevel->init();
}

void Engine::run() {
    float lastFrame = static_cast<float>(glfwGetTime());
    float fpsTimer = 0.0f;
    int frameCount = 0;
    float displayFps = 60.0f;
    float displayMs = 16.6f;

    while (!glfwWindowShouldClose(window)) {
        float currentFrame = static_cast<float>(glfwGetTime());
        float deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        fpsTimer += deltaTime;
        frameCount++;
        if (fpsTimer >= 0.25f) {
            displayFps = static_cast<float>(frameCount) / fpsTimer;
            displayMs = (displayFps > 0.001f) ? (1000.0f / displayFps) : 0.0f;
            fpsTimer = 0.0f;
            frameCount = 0;

            char winTitle[128];
            std::snprintf(winTitle, sizeof(winTitle), "%s | FPS: %.0f (%.1f ms)", title, displayFps, displayMs);
            glfwSetWindowTitle(window, winTitle);
        }

        static bool key1Pressed = false;
        static bool key2Pressed = false;
        static bool key3Pressed = false;
        static bool key4Pressed = false;

        if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) {
            if (!key1Pressed) { changeLevel(new Level1(this)); key1Pressed = true; }
        } else key1Pressed = false;

        if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS) {
            if (!key2Pressed) { changeLevel(new Level2(this)); key2Pressed = true; }
        } else key2Pressed = false;

        if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS) {
            if (!key3Pressed) { changeLevel(new Level3(this)); key3Pressed = true; }
        } else key3Pressed = false;

        if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS) {
            if (!key4Pressed) { changeLevel(new Level4(this)); key4Pressed = true; }
        } else key4Pressed = false;

        if (glfwGetKey(window, GLFW_KEY_H) == GLFW_PRESS) {
            if (!hKeyPressed) { hud.toggleVisible(); hKeyPressed = true; }
        } else hKeyPressed = false;

        if (currentLevel) {
            currentLevel->processInput(window, deltaTime);
            currentLevel->update(deltaTime);
            currentLevel->render();
        }

        int winW = static_cast<int>(width);
        int winH = static_cast<int>(height);
        glfwGetWindowSize(window, &winW, &winH);
        if (winW > 0 && winH > 0) {
            std::string hint;
            if (auto* lvl4 = dynamic_cast<Level4*>(currentLevel)) {
                hint = lvl4->getHudHint();
            }
            hud.render(winW, winH, displayFps, displayMs, currentLevelIndex, currentLevelName, hint);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
}

void Engine::framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    (void)window;
    glViewport(0, 0, width, height);
}