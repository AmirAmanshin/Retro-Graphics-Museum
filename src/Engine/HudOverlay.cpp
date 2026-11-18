#include "Engine/HudOverlay.hpp"
#include <array>
#include <vector>
#include <cstdio>
#include <cstdint>
#include <algorithm>
#include <glm/gtc/matrix_transform.hpp>

HudOverlay::HudOverlay()
    : hudShader(nullptr), quadVAO(0), quadVBO(0), fontTexture(0), visible(true) {}

HudOverlay::~HudOverlay() {
    if (hudShader) delete hudShader;
    if (quadVAO) glDeleteVertexArrays(1, &quadVAO);
    if (quadVBO) glDeleteBuffers(1, &quadVBO);
    if (fontTexture) glDeleteTextures(1, &fontTexture);
}

static std::array<uint8_t, 8> GetGlyph8x8(char c) {
    if (c >= 'a' && c <= 'z') c = static_cast<char>(c - 'a' + 'A');
    switch (c) {
        case '0': return {0x3C,0x66,0x6E,0x76,0x66,0x66,0x3C,0x00};
        case '1': return {0x18,0x38,0x18,0x18,0x18,0x18,0x7E,0x00};
        case '2': return {0x3C,0x66,0x06,0x1C,0x30,0x66,0x7E,0x00};
        case '3': return {0x3C,0x66,0x06,0x1C,0x06,0x66,0x3C,0x00};
        case '4': return {0x0E,0x1E,0x36,0x66,0x7F,0x06,0x0F,0x00};
        case '5': return {0x7E,0x60,0x7C,0x06,0x06,0x66,0x3C,0x00};
        case '6': return {0x1C,0x30,0x60,0x7C,0x66,0x66,0x3C,0x00};
        case '7': return {0x7E,0x66,0x0C,0x18,0x18,0x18,0x18,0x00};
        case '8': return {0x3C,0x66,0x66,0x3C,0x66,0x66,0x3C,0x00};
        case '9': return {0x3C,0x66,0x66,0x3E,0x06,0x0C,0x38,0x00};
        case 'A': return {0x18,0x3C,0x66,0x66,0x7E,0x66,0x66,0x00};
        case 'B': return {0x7C,0x66,0x66,0x7C,0x66,0x66,0x7C,0x00};
        case 'C': return {0x3C,0x66,0x60,0x60,0x60,0x66,0x3C,0x00};
        case 'D': return {0x78,0x6C,0x66,0x66,0x66,0x6C,0x78,0x00};
        case 'E': return {0x7E,0x60,0x60,0x7C,0x60,0x60,0x7E,0x00};
        case 'F': return {0x7E,0x60,0x60,0x7C,0x60,0x60,0x60,0x00};
        case 'G': return {0x3C,0x66,0x60,0x6E,0x66,0x66,0x3E,0x00};
        case 'H': return {0x66,0x66,0x66,0x7E,0x66,0x66,0x66,0x00};
        case 'I': return {0x3C,0x18,0x18,0x18,0x18,0x18,0x3C,0x00};
        case 'J': return {0x1E,0x0C,0x0C,0x0C,0x6C,0x6C,0x38,0x00};
        case 'K': return {0x66,0x6C,0x78,0x70,0x78,0x6C,0x66,0x00};
        case 'L': return {0x60,0x60,0x60,0x60,0x60,0x60,0x7E,0x00};
        case 'M': return {0x63,0x77,0x7F,0x6B,0x63,0x63,0x63,0x00};
        case 'N': return {0x66,0x76,0x7E,0x7E,0x6E,0x66,0x66,0x00};
        case 'O': return {0x3C,0x66,0x66,0x66,0x66,0x66,0x3C,0x00};
        case 'P': return {0x7C,0x66,0x66,0x7C,0x60,0x60,0x60,0x00};
        case 'Q': return {0x3C,0x66,0x66,0x66,0x6A,0x6C,0x36,0x00};
        case 'R': return {0x7C,0x66,0x66,0x7C,0x6C,0x66,0x66,0x00};
        case 'S': return {0x3C,0x66,0x60,0x3C,0x06,0x66,0x3C,0x00};
        case 'T': return {0x7E,0x18,0x18,0x18,0x18,0x18,0x18,0x00};
        case 'U': return {0x66,0x66,0x66,0x66,0x66,0x66,0x3C,0x00};
        case 'V': return {0x66,0x66,0x66,0x66,0x66,0x3C,0x18,0x00};
        case 'W': return {0x63,0x63,0x63,0x6B,0x7F,0x77,0x63,0x00};
        case 'X': return {0x66,0x66,0x3C,0x18,0x3C,0x66,0x66,0x00};
        case 'Y': return {0x66,0x66,0x66,0x3C,0x18,0x18,0x18,0x00};
        case 'Z': return {0x7E,0x06,0x0C,0x18,0x30,0x60,0x7E,0x00};
        case ':': return {0x00,0x18,0x18,0x00,0x18,0x18,0x00,0x00};
        case '.': return {0x00,0x00,0x00,0x00,0x00,0x18,0x18,0x00};
        case '-': return {0x00,0x00,0x00,0x7E,0x00,0x00,0x00,0x00};
        case '/': return {0x06,0x0C,0x18,0x30,0x60,0x40,0x00,0x00};
        case '[': return {0x3C,0x30,0x30,0x30,0x30,0x30,0x3C,0x00};
        case ']': return {0x3C,0x0C,0x0C,0x0C,0x0C,0x0C,0x3C,0x00};
        case '(': return {0x0C,0x18,0x30,0x30,0x30,0x18,0x0C,0x00};
        case ')': return {0x30,0x18,0x0C,0x0C,0x0C,0x18,0x30,0x00};
        case '!': return {0x18,0x18,0x18,0x18,0x18,0x00,0x18,0x00};
        case ',': return {0x00,0x00,0x00,0x00,0x00,0x18,0x18,0x30};
        case '+': return {0x00,0x18,0x18,0x7E,0x18,0x18,0x00,0x00};
        case '>': return {0x30,0x18,0x0C,0x06,0x0C,0x18,0x30,0x00};
        case '<': return {0x0C,0x18,0x30,0x60,0x30,0x18,0x0C,0x00};
        case '&': return {0x38,0x6C,0x38,0x76,0xDC,0xCC,0x76,0x00};
        case '*': return {0x00,0x66,0x3C,0xFF,0x3C,0x66,0x00,0x00};
        case '_': return {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF};
        default:  return {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
    }
}

void HudOverlay::createFontTexture() {
    std::vector<uint8_t> atlas(128 * 64, 0);
    for (int code = 32; code < 128; ++code) {
        int idx = code - 32;
        int gx = (idx % 16) * 8;
        int gy = (idx / 16) * 8;
        auto rows = GetGlyph8x8(static_cast<char>(code));
        for (int r = 0; r < 8; ++r) {
            for (int c = 0; c < 8; ++c) {
                if (rows[r] & (1 << (7 - c))) {
                    atlas[(gy + r) * 128 + (gx + c)] = 255;
                }
            }
        }
    }

    glGenTextures(1, &fontTexture);
    glBindTexture(GL_TEXTURE_2D, fontTexture);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, 128, 64, 0, GL_RED, GL_UNSIGNED_BYTE, atlas.data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

void HudOverlay::init() {
    hudShader = new Shader("shaders/hud.vert", "shaders/hud.frag");
    createFontTexture();

    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);
    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, 6 * 4 * sizeof(float), nullptr, GL_DYNAMIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);
}

void HudOverlay::drawRoundedBox(float x, float y, float w, float h, float radius,
                                glm::vec4 fill, glm::vec4 border, const glm::mat4& proj) {
    float verts[] = {
        x,     y,      0.0f, 0.0f,
        x + w, y,      1.0f, 0.0f,
        x + w, y + h,  1.0f, 1.0f,
        x,     y,      0.0f, 0.0f,
        x + w, y + h,  1.0f, 1.0f,
        x,     y + h,  0.0f, 1.0f
    };

    hudShader->use();
    hudShader->setMat4("projection", proj);
    hudShader->setInt("mode", 0);
    hudShader->setVec2("boxPos", glm::vec2(x, y));
    hudShader->setVec2("boxSize", glm::vec2(w, h));
    hudShader->setFloat("cornerRadius", radius);
    hudShader->setVec4("fillColor", fill);
    hudShader->setVec4("borderColor", border);

    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(verts), verts);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}

void HudOverlay::drawText(const std::string& text, float x, float y, float scale,
                          glm::vec4 color, const glm::mat4& proj) {
    hudShader->use();
    hudShader->setMat4("projection", proj);
    hudShader->setInt("mode", 1);
    hudShader->setInt("fontTex", 0);
    hudShader->setVec4("textColor", color);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, fontTexture);
    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);

    float adv = 8.0f * scale;
    float cx = x;

    for (char raw : text) {
        int code = static_cast<unsigned char>(raw);
        if (code < 32 || code >= 128) code = 32;
        int idx = code - 32;
        float u0 = (idx % 16) * (8.0f / 128.0f);
        float v0 = (idx / 16) * (8.0f / 64.0f);
        float u1 = u0 + (8.0f / 128.0f);
        float v1 = v0 + (8.0f / 64.0f);

        float w = 8.0f * scale;
        float h = 8.0f * scale;
        float verts[] = {
            cx,     y,      u0, v0,
            cx + w, y,      u1, v0,
            cx + w, y + h,  u1, v1,
            cx,     y,      u0, v0,
            cx + w, y + h,  u1, v1,
            cx,     y + h,  u0, v1
        };

        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(verts), verts);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        cx += adv;
    }

    glBindVertexArray(0);
}

void HudOverlay::render(int windowWidth, int windowHeight, float fps, float frameMs,
                        int levelIndex, const std::string& levelLabel,
                        const std::string& statusHint,
                        bool canonicalFpsMode, float targetFps) {
    if (!visible || !hudShader) return;

    GLboolean depthWasEnabled = glIsEnabled(GL_DEPTH_TEST);
    GLboolean blendWasEnabled = glIsEnabled(GL_BLEND);
    GLint prevPolygonMode[2] = { GL_FILL, GL_FILL };
    glGetIntegerv(GL_POLYGON_MODE, prevPolygonMode);

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glm::mat4 proj = glm::ortho(0.0f, static_cast<float>(windowWidth),
                                static_cast<float>(windowHeight), 0.0f);

    char fpsLine[96];
    std::snprintf(fpsLine, sizeof(fpsLine), "FPS: %.0f (%.1f MS) [%s: %.0f]",
                  fps, frameMs, canonicalFpsMode ? "CANON" : "STD", targetFps);

    std::vector<std::string> lines;
    lines.push_back("[WASD] MOVE   [SPACE] JUMP");
    if (levelIndex == 4) {
        lines.push_back("[TAB] HOLD PDA  [SHIFT] SPRINT");
        lines.push_back("[F] FLASHLIGHT  [E] INTERACT");
        lines.push_back("[1-4] LEVELS    [N] NOCLIP");
    } else if (levelIndex == 3) {
        lines.push_back("[LMB] SHOTGUN   [R] RELOAD");
        lines.push_back("[F] NOCLIP      [1-4] LEVELS");
    } else {
        lines.push_back("[F] NOCLIP      [1-4] LEVELS");
    }
    lines.push_back("[C] FPS MODE    [H] HIDE HUD");
    lines.push_back("[ESC] QUIT");

    float maxContentW = static_cast<float>(std::string(fpsLine).size()) * 8.0f * 1.85f;
    maxContentW = std::max(maxContentW, static_cast<float>(levelLabel.size()) * 8.0f * 1.35f);
    for (const auto& l : lines) {
        maxContentW = std::max(maxContentW, static_cast<float>(l.size()) * 8.0f * 1.25f);
    }
    if (!statusHint.empty()) {
        maxContentW = std::max(maxContentW, static_cast<float>(statusHint.size()) * 8.0f * 1.25f);
    }

    float boxW = std::max(315.0f, maxContentW + 28.0f);
    float boxH = 68.0f + static_cast<float>(lines.size()) * 18.0f + (statusHint.empty() ? 6.0f : 26.0f);

    drawRoundedBox(16.0f, 16.0f, boxW, boxH, 12.0f,
                   glm::vec4(0.05f, 0.06f, 0.08f, 0.72f),
                   glm::vec4(0.85f, 0.22f, 0.22f, 0.55f), proj);

    drawText(fpsLine, 30.0f, 30.0f, 1.85f, glm::vec4(1.0f, 0.92f, 0.35f, 0.98f), proj);
    drawText(levelLabel, 30.0f, 54.0f, 1.35f, glm::vec4(0.95f, 0.35f, 0.35f, 0.95f), proj);

    float curY = 78.0f;
    for (const auto& l : lines) {
        drawText(l, 30.0f, curY, 1.25f, glm::vec4(0.86f, 0.88f, 0.92f, 0.92f), proj);
        curY += 18.0f;
    }

    if (!statusHint.empty()) {
        drawText(statusHint, 30.0f, curY + 4.0f, 1.25f, glm::vec4(0.45f, 0.95f, 0.65f, 0.98f), proj);
    }

    glPolygonMode(GL_FRONT_AND_BACK, prevPolygonMode[0]);
    if (!blendWasEnabled) glDisable(GL_BLEND);
    if (depthWasEnabled) glEnable(GL_DEPTH_TEST);
}