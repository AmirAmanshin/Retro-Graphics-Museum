#include "Engine/Engine.hpp"
#include <iostream>
#include <thread>
#include <chrono>

Engine::Engine(unsigned int w, unsigned int h, const char* t) 
    : width(w), height(h), title(t), window(nullptr), currentLevel(nullptr) {}

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
    return true;
}

void Engine::changeLevel(Level* newLevel) {
    if (currentLevel) delete currentLevel;
    currentLevel = newLevel;
    currentLevel->init();
}

void Engine::run() {
    const double targetFPS = 15.0; 
    const double frameDelay = 1.0 / targetFPS;
    float lastFrame = 0.0f;

    while (!glfwWindowShouldClose(window)) {
        float currentFrame = glfwGetTime();
        float deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        if (currentLevel) {
            currentLevel->processInput(window, deltaTime);
            currentLevel->update(deltaTime);
            currentLevel->render();
        }

        glfwSwapBuffers(window);
        glfwPollEvents();

        double timeSpent = glfwGetTime() - currentFrame;
        if (timeSpent < frameDelay) {
            std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>((frameDelay - timeSpent) * 1000.0)));
        }
    }
}

void Engine::framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}