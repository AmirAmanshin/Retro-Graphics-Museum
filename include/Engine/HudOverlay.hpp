#pragma once
#include <string>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include "Engine/Shader.hpp"

class HudOverlay {
public:
    HudOverlay();
    ~HudOverlay();

    void init();
    void toggleVisible() { visible = !visible; }
    bool isVisible() const { return visible; }

    void render(int windowWidth, int windowHeight, float fps, float frameMs,
                int levelIndex, const std::string& levelLabel,
                const std::string& statusHint = "",
                bool canonicalFpsMode = true, float targetFps = 60.0f);

    void drawRoundedBox(float x, float y, float w, float h, float radius,
                        glm::vec4 fill, glm::vec4 border, const glm::mat4& proj);
    void drawText(const std::string& text, float x, float y, float scale,
                  glm::vec4 color, const glm::mat4& proj);

private:
    Shader* hudShader;
    GLuint quadVAO;
    GLuint quadVBO;
    GLuint fontTexture;
    bool visible;

    void createFontTexture();
};