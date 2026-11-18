#include "Engine/Engine.hpp"
#include <iostream>
#include <cstdio>
#include <chrono>
#include <thread>

#include "Levels/Level1.hpp"
#include "Levels/Level2.hpp"
#include "Levels/Level3.hpp"
#include "Levels/Level4.hpp"

Engine::Engine(unsigned int w, unsigned int h, const char* t)
    : width(w), height(h), title(t), window(nullptr),
      currentLevel(nullptr), pendingLevel(nullptr),
      currentLevelIndex(0), currentLevelName("RETRO GRAPHICS MUSEUM"),
      hKeyPressed(false), useCanonicalFps(true), cKeyPressed(false) {}

Engine::~Engine() {
    if (pendingLevel) delete pendingLevel;
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
    glfwSwapInterval(0);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) return false;

    glEnable(GL_DEPTH_TEST);
    hud.init();
    return true;
}

void Engine::requestQuit() {
    if (window) {
        glfwSetWindowShouldClose(window, true);
    }
}

float Engine::getActiveTargetFps() const {
    if (!useCanonicalFps) {
        return standardFps;
    }
    switch (currentLevelIndex) {
        case 1:  return level1CanonicalFps;
        case 2:  return level2CanonicalFps;
        case 3:  return level3CanonicalFps;
        case 4:  return level4CanonicalFps;
        default: return menuCanonicalFps;
    }
}

void Engine::changeLevel(Level* newLevel) {
    if (!currentLevel) {
        applyLevelChange(newLevel);
    } else {
        if (pendingLevel) delete pendingLevel;
        pendingLevel = newLevel;
    }
}

void Engine::applyLevelChange(Level* newLevel) {
    if (currentLevel) delete currentLevel;
    currentLevel = newLevel;

    if (dynamic_cast<Level4*>(currentLevel)) {
        currentLevelIndex = 4;
        currentLevelName = "ERA 4: HORROR (PS1 / BLINN-PHONG)";
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

    // В игровых уровнях захватываем курсор, в меню и титрах — освобождаем
    if (currentLevelIndex >= 1 && currentLevelIndex <= 4) {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    } else {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }

    currentLevel->init();
}

void Engine::run() {
    double lastFrame = glfwGetTime();
    float fpsTimer = 0.0f;
    int frameCount = 0;
    float displayFps = 60.0f;
    float displayMs = 16.6f;

    while (!glfwWindowShouldClose(window)) {
        if (pendingLevel) {
            Level* next = pendingLevel;
            pendingLevel = nullptr;
            applyLevelChange(next);
        }

        double frameStart = glfwGetTime();
        float deltaTime = static_cast<float>(frameStart - lastFrame);
        lastFrame = frameStart;

        fpsTimer += deltaTime;
        frameCount++;
        if (fpsTimer >= 0.25f) {
            displayFps = static_cast<float>(frameCount) / fpsTimer;
            displayMs = (displayFps > 0.001f) ? (1000.0f / displayFps) : 0.0f;
            fpsTimer = 0.0f;
            frameCount = 0;

            char winTitle[160];
            std::snprintf(winTitle, sizeof(winTitle), "%s | FPS: %.0f (%.1f ms) [%s: %.0f]",
                          title, displayFps, displayMs,
                          useCanonicalFps ? "CANON" : "STD", getActiveTargetFps());
            glfwSetWindowTitle(window, winTitle);
        }

        // Быстрый выбор уровней 1-4 и скрытие HUD работают только во время игры
        if (currentLevelIndex > 0) {
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
        }

        if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS) {
            if (!cKeyPressed) {
                useCanonicalFps = !useCanonicalFps;
                cKeyPressed = true;
            }
        } else cKeyPressed = false;

        if (currentLevel) {
            currentLevel->processInput(window, deltaTime);
            currentLevel->update(deltaTime);
            currentLevel->render();
        }

        int winW = static_cast<int>(width);
        int winH = static_cast<int>(height);
        glfwGetWindowSize(window, &winW, &winH);
        if (winW > 0 && winH > 0 && currentLevelIndex > 0) {
            std::string hint;
            if (auto* lvl4 = dynamic_cast<Level4*>(currentLevel)) {
                hint = lvl4->getHudHint();
            }
            hud.render(winW, winH, displayFps, displayMs, currentLevelIndex, currentLevelName,
                       hint, useCanonicalFps, getActiveTargetFps());
        }

        glfwSwapBuffers(window);
        glfwPollEvents();

        float targetFps = getActiveTargetFps();
        if (targetFps > 0.0f) {
            double targetFrameTime = 1.0 / static_cast<double>(targetFps);
            double elapsed = glfwGetTime() - frameStart;
            double remaining = targetFrameTime - elapsed;

            if (remaining > 0.002) {
                std::this_thread::sleep_for(std::chrono::duration<double>(remaining - 0.0015));
            }
            while (glfwGetTime() - frameStart < targetFrameTime) {
            }
        }
    }
}

void Engine::framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    (void)window;
    glViewport(0, 0, width, height);
}