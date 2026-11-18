#include "Levels/MainMenuLevel.hpp"
#include "Levels/Level1.hpp"
#include "Levels/Level2.hpp"
#include "Levels/Level3.hpp"
#include "Levels/Level4.hpp"
#include "Levels/CreditsLevel.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>
#include <algorithm>

MainMenuLevel::MainMenuLevel(Engine* engine)
    : gameEngine(engine), selectedIndex(0), selectedEra(1), animTime(0.0f),
      upPressed(false), downPressed(false), leftPressed(false), rightPressed(false),
      enterPressed(false), mousePressed(false),
      escReleased(false), enterReleased(false) {}

void MainMenuLevel::init() {
    selectedIndex = 0;
    animTime = 0.0f;
    escReleased = false;
    enterReleased = false;
}

void MainMenuLevel::launchEra(int era) {
    switch (era) {
        case 1: gameEngine->changeLevel(new Level1(gameEngine)); break;
        case 2: gameEngine->changeLevel(new Level2(gameEngine)); break;
        case 3: gameEngine->changeLevel(new Level3(gameEngine)); break;
        case 4: gameEngine->changeLevel(new Level4(gameEngine)); break;
        default: gameEngine->changeLevel(new Level1(gameEngine)); break;
    }
}

void MainMenuLevel::activateItem(int index, int dir) {
    switch (index) {
        case 0: // START EXHIBITION
            launchEra(1);
            break;
        case 1: // SELECT ERA (1-4)
            if (dir == 0) {
                launchEra(selectedEra);
            } else {
                selectedEra += dir;
                if (selectedEra > 4) selectedEra = 1;
                if (selectedEra < 1) selectedEra = 4;
            }
            break;
        case 2: // FPS MODE
            gameEngine->toggleCanonicalFps();
            break;
        case 3: // CREDITS
            gameEngine->changeLevel(new CreditsLevel(gameEngine));
            break;
        case 4: // EXIT
            gameEngine->requestQuit();
            break;
        default:
            break;
    }
}

void MainMenuLevel::processInput(GLFWwindow* window, float deltaTime) {
    (void)deltaTime;
    const int itemCount = 5;

    // Выход из программы по ESC только после того, как клавиша была отпущена
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_RELEASE) {
        escReleased = true;
    } else if (escReleased && glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        gameEngine->requestQuit();
        return;
    }

    bool upDown = (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) ||
                  (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS);
    if (upDown) {
        if (!upPressed) {
            selectedIndex = (selectedIndex - 1 + itemCount) % itemCount;
            upPressed = true;
        }
    } else {
        upPressed = false;
    }

    bool downDown = (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) ||
                    (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS);
    if (downDown) {
        if (!downPressed) {
            selectedIndex = (selectedIndex + 1) % itemCount;
            downPressed = true;
        }
    } else {
        downPressed = false;
    }

    bool leftDown = (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) ||
                    (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS);
    if (leftDown) {
        if (!leftPressed) {
            if (selectedIndex == 1 || selectedIndex == 2) activateItem(selectedIndex, -1);
            leftPressed = true;
        }
    } else {
        leftPressed = false;
    }

    bool rightDown = (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) ||
                     (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS);
    if (rightDown) {
        if (!rightPressed) {
            if (selectedIndex == 1 || selectedIndex == 2) activateItem(selectedIndex, 1);
            rightPressed = true;
        }
    } else {
        rightPressed = false;
    }

    bool enterDown = (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS) ||
                     (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS);
    if (!enterDown) {
        enterReleased = true;
        enterPressed = false;
    } else if (enterReleased && !enterPressed) {
        enterPressed = true;
        activateItem(selectedIndex, 0);
        return;
    }

    // Поддержка наведения и клика мышью
    int winW = 1280, winH = 720;
    glfwGetWindowSize(window, &winW, &winH);
    double mx = 0.0, my = 0.0;
    glfwGetCursorPos(window, &mx, &my);

    float panelW = 580.0f;
    float panelX = (static_cast<float>(winW) - panelW) * 0.5f;
    float startY = static_cast<float>(winH) * 0.38f;
    float itemH = 48.0f;
    float gap = 12.0f;

    bool lmbDown = (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS);
    for (int i = 0; i < itemCount; ++i) {
        float iy = startY + i * (itemH + gap);
        if (mx >= panelX + 24.0f && mx <= panelX + panelW - 24.0f &&
            my >= iy && my <= iy + itemH) {
            selectedIndex = i;
            if (lmbDown && !mousePressed) {
                mousePressed = true;
                activateItem(i, (i == 1) ? 0 : 1);
                return;
            }
        }
    }
    if (!lmbDown) mousePressed = false;
}

void MainMenuLevel::update(float deltaTime) {
    animTime += deltaTime;
}

void MainMenuLevel::render() {
    int winW = 1280, winH = 720;
    glfwGetFramebufferSize(glfwGetCurrentContext(), &winW, &winH);
    if (winW <= 0 || winH <= 0) return;

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glClearColor(0.03f, 0.035f, 0.05f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    float w = static_cast<float>(winW);
    float h = static_cast<float>(winH);
    glm::mat4 proj = glm::ortho(0.0f, w, h, 0.0f);

    HudOverlay& hud = gameEngine->getHud();

    // Анимированная ретро-сетка на фоне
    float gridStep = 64.0f;
    float shift = std::fmod(animTime * 22.0f, gridStep);
    glm::vec4 gridCol(0.14f, 0.28f, 0.24f, 0.16f);
    for (float gx = shift; gx < w; gx += gridStep) {
        hud.drawRoundedBox(gx, 0.0f, 1.0f, h, 0.0f, gridCol, gridCol, proj);
    }
    for (float gy = shift; gy < h; gy += gridStep) {
        hud.drawRoundedBox(0.0f, gy, w, 1.0f, 0.0f, gridCol, gridCol, proj);
    }

    // Заголовок музея
    std::string mainTitle = "RETRO GRAPHICS MUSEUM";
    std::string subTitle  = "EVOLUTION OF 3D ENGINE RENDERING";
    float titleScale = 3.2f;
    float subScale   = 1.5f;
    float titleW = mainTitle.size() * 8.0f * titleScale;
    float subW   = subTitle.size() * 8.0f * subScale;

    hud.drawText(mainTitle, (w - titleW) * 0.5f, h * 0.14f, titleScale,
                 glm::vec4(0.96f, 0.88f, 0.35f, 1.0f), proj);
    hud.drawText(subTitle, (w - subW) * 0.5f, h * 0.14f + 36.0f, subScale,
                 glm::vec4(0.55f, 0.85f, 0.72f, 0.92f), proj);

    // Центральная панель меню
    float panelW = 580.0f;
    float panelH = 340.0f;
    float panelX = (w - panelW) * 0.5f;
    float panelY = h * 0.35f;

    hud.drawRoundedBox(panelX, panelY, panelW, panelH, 14.0f,
                       glm::vec4(0.05f, 0.06f, 0.09f, 0.88f),
                       glm::vec4(0.85f, 0.25f, 0.25f, 0.65f), proj);

    const char* eraLabels[4] = {
        "1: 3D WIREFRAME",
        "2: 2.5D MAZE",
        "3: 3D + BILLBOARDS",
        "4: PS1 HORROR"
    };

    std::vector<std::string> items = {
        "START EXHIBITION",
        std::string("SELECT ERA: < ") + eraLabels[selectedEra - 1] + " >",
        std::string("FPS CAP: ") + (gameEngine->isCanonicalFps() ? "CANONICAL (ERA)" : "STANDARD (144)"),
        "CREDITS",
        "EXIT MUSEUM"
    };

    float startY = h * 0.38f;
    float itemH  = 48.0f;
    float gap    = 12.0f;

    for (int i = 0; i < static_cast<int>(items.size()); ++i) {
        float iy = startY + i * (itemH + gap);
        bool active = (i == selectedIndex);

        glm::vec4 boxFill = active
            ? glm::vec4(0.24f, 0.10f, 0.12f, 0.95f)
            : glm::vec4(0.08f, 0.09f, 0.13f, 0.75f);
        glm::vec4 boxBorder = active
            ? glm::vec4(0.98f, 0.85f, 0.32f, 0.95f)
            : glm::vec4(0.25f, 0.28f, 0.35f, 0.45f);

        hud.drawRoundedBox(panelX + 24.0f, iy, panelW - 48.0f, itemH, 8.0f, boxFill, boxBorder, proj);

        std::string label = (active ? "> " : "  ") + items[i];
        glm::vec4 textCol = active
            ? glm::vec4(1.0f, 0.94f, 0.45f, 1.0f)
            : glm::vec4(0.84f, 0.86f, 0.90f, 0.90f);

        hud.drawText(label, panelX + 42.0f, iy + 16.0f, 1.85f, textCol, proj);
    }

    // Подсказка внизу экрана
    std::string footer = "[W/S] NAVIGATE   [A/D] CHANGE   [ENTER] SELECT   [C] FPS MODE";
    float footerW = footer.size() * 8.0f * 1.3f;
    hud.drawText(footer, (w - footerW) * 0.5f, h - 38.0f, 1.3f,
                 glm::vec4(0.62f, 0.66f, 0.74f, 0.85f), proj);
}