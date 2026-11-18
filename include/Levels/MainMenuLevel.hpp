#pragma once
#include <vector>
#include <string>
#include "Level.hpp"
#include "Engine/Engine.hpp"

class MainMenuLevel : public Level {
public:
    MainMenuLevel(Engine* engine);
    ~MainMenuLevel() override = default;

    void init() override;
    void processInput(GLFWwindow* window, float deltaTime) override;
    void update(float deltaTime) override;
    void render() override;

private:
    Engine* gameEngine;
    int selectedIndex;
    int selectedEra; // 1..4
    float animTime;

    bool upPressed;
    bool downPressed;
    bool leftPressed;
    bool rightPressed;
    bool enterPressed;
    bool mousePressed;

    bool escReleased;
    bool enterReleased;

    void activateItem(int index, int dir = 1);
    void launchEra(int era);
};