#pragma once
#include <vector>
#include <string>
#include <glm/glm.hpp>
#include "Level.hpp"
#include "Engine/Engine.hpp"

struct CreditLine {
    std::string text;
    float scale;
    glm::vec4 color;
    float spacingAfter;
};

class CreditsLevel : public Level {
public:
    CreditsLevel(Engine* engine);
    ~CreditsLevel() override = default;

    void init() override;
    void processInput(GLFWwindow* window, float deltaTime) override;
    void update(float deltaTime) override;
    void render() override;

private:
    Engine* gameEngine;
    std::vector<CreditLine> lines;
    float scrollY;
    float totalHeight;
    bool fastForward;
    bool exitKeyReleased;
};