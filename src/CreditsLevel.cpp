#include "Levels/CreditsLevel.hpp"
#include "Levels/MainMenuLevel.hpp"
#include <glm/gtc/matrix_transform.hpp>

CreditsLevel::CreditsLevel(Engine* engine)
    : gameEngine(engine), scrollY(0.0f), totalHeight(0.0f),
      fastForward(false), exitKeyReleased(false) {}

void CreditsLevel::init() {
    lines.clear();

    glm::vec4 gold  = glm::vec4(0.98f, 0.88f, 0.35f, 1.0f);
    glm::vec4 red   = glm::vec4(0.95f, 0.35f, 0.35f, 1.0f);
    glm::vec4 cyan  = glm::vec4(0.45f, 0.92f, 0.78f, 1.0f);
    glm::vec4 white = glm::vec4(0.88f, 0.90f, 0.94f, 0.95f);
    glm::vec4 muted = glm::vec4(0.60f, 0.64f, 0.72f, 0.88f);

    auto add = [&](const std::string& t, float scale, glm::vec4 col, float space) {
        lines.push_back({ t, scale, col, space });
    };

    // =========================================================================
    // 1. ВСТУПЛЕНИЕ И ОБЗОР ПРОЕКТА
    // =========================================================================
    add("RETRO GRAPHICS MUSEUM", 3.0f, gold, 18.0f);
    add("INTERACTIVE EVOLUTION OF 3D REAL-TIME RENDERING", 1.5f, cyan, 12.0f);
    add("COMPUTER GRAPHICS FINAL PROJECT", 1.35f, muted, 64.0f);

    // =========================================================================
    // 2. КОМАНДА РАЗРАБОТКИ
    // =========================================================================
    add("DEVELOPMENT TEAM", 2.05f, red, 24.0f);

    add("AMIR", 1.75f, gold, 10.0f);
    add("CORE ENGINE ARCHITECTURE, LEVEL ABSTRACTION & CAMERA SYSTEM", 1.3f, white, 8.0f);
    add("ERA 1 & ERA 2 IMPLEMENTATION, HUD OVERLAY & MAIN MENU", 1.3f, white, 8.0f);
    add("ERA 3 SHOTGUN MECHANICS, ERA 4 PS1 POST-PROCESSING & STALKER AI", 1.3f, white, 34.0f);

    add("NURALY", 1.75f, gold, 10.0f);
    add("ERA 3 & ERA 4 ENVIRONMENT DESIGN & BLINN-PHONG LIGHTING", 1.3f, white, 8.0f);
    add("PROCEDURAL MAZE GENERATOR, PROPS, COMPASS & AUDIO INTEGRATION", 1.3f, white, 8.0f);
    add("ATMOSPHERIC SOUND TRIGGERS & INTERACTIVE LEVEL PUZZLES", 1.3f, white, 60.0f);

    // =========================================================================
    // 3. ЭКСПОЗИЦИЯ ПО ЭПОХАМ (УРОВНИ 1 - 4)
    // =========================================================================
    add("EXHIBITION HALLS", 2.05f, red, 24.0f);

    add("ERA 1: EARLY 3D WIREFRAME (1980S)", 1.55f, cyan, 8.0f);
    add("VECTOR LINE RENDERING, GRID-BASED MOVEMENT & POLYGON OFFSET", 1.25f, white, 26.0f);

    add("ERA 2: 2.5D TEXTURED MAZE (EARLY 1990S)", 1.55f, cyan, 8.0f);
    add("WOLFENSTEIN-STYLE HORIZONTAL PROJECTION, PIXEL ART & COMPASS", 1.25f, white, 26.0f);

    add("ERA 3: TRUE 3D & BILLBOARDS (LATE 1990S)", 1.55f, cyan, 8.0f);
    add("FULL 3D CAMERA PITCH, JUMP PHYSICS, ALPHA SPRITES & DOUBLE-BARREL", 1.25f, white, 26.0f);

    add("ERA 4: PS1 SURVIVAL HORROR (1996-1999)", 1.55f, cyan, 8.0f);
    add("240X180 FBO, VERTEX SNAPPING, BAYER DITHERING & COLOR BANDING", 1.25f, white, 8.0f);
    add("SPOTLIGHT FLASHLIGHT, PDA MOTION RADAR & BFS PATHFINDING AI", 1.25f, white, 60.0f);

    // =========================================================================
    // 4. OPEN-SOURCE LIBRARIES (ИЗ CREDITS.MD)
    // =========================================================================
    add("OPEN-SOURCE LIBRARIES", 2.05f, red, 24.0f);

    add("GLFW 3  -  MARCUS GEELNARD, CAMILLA LOWY  [ZLIB/LIBPNG]", 1.4f, cyan, 8.0f);
    add("WINDOW CREATION, OPENGL 3.3 CONTEXT, KEYBOARD & MOUSE INPUT", 1.2f, white, 22.0f);

    add("GLAD  -  DAVID HERBERTH  [MIT / PUBLIC DOMAIN]", 1.4f, cyan, 8.0f);
    add("OPENGL 3.3 CORE PROFILE FUNCTION LOADER", 1.2f, white, 22.0f);

    add("GLM  -  G-TRUC CREATION / CHRISTOPHE RICCIO  [MIT]", 1.4f, cyan, 8.0f);
    add("VECTOR & MATRIX MATH (VEC2-VEC4, MAT4, PERSPECTIVE, ORTHO)", 1.2f, white, 22.0f);

    add("STB_IMAGE  -  SEAN BARRETT  [MIT / PUBLIC DOMAIN]", 1.4f, cyan, 8.0f);
    add("PNG/JPG TEXTURE DECODING WITH ALPHA CHANNEL SUPPORT", 1.2f, white, 22.0f);

    add("MINIAUDIO  -  DAVID REID  [MIT-0 / PUBLIC DOMAIN]", 1.4f, cyan, 8.0f);
    add("CROSS-PLATFORM AUDIO PLAYBACK FOR MUSIC LOOPS & SFX", 1.2f, white, 60.0f);

    // =========================================================================
    // 5. TECHNICAL & MATHEMATICAL REFERENCES (ИЗ CREDITS.MD)
    // =========================================================================
    add("TECHNICAL & MATHEMATICAL REFERENCES", 2.05f, red, 24.0f);

    add("LEARNOPENGL BY JOEY DE VRIES  [CC BY-NC 4.0]", 1.4f, cyan, 8.0f);
    add("OPENGL 3.3 VAO/VBO SETUP, GLSL PIPELINE, EULER CAMERA & BLINN-PHONG", 1.2f, white, 22.0f);

    add("INIGO QUILEZ - 2D DISTANCE FUNCTIONS", 1.4f, cyan, 8.0f);
    add("2D ROUNDED BOX SIGNED DISTANCE FUNCTION (ROUNDEDBOXSDF) IN HUD.FRAG", 1.2f, white, 22.0f);

    add("DEVELOPMENT & DEBUGGING WORKFLOW", 1.4f, cyan, 8.0f);
    add("C++17, CMAKE & GEMINI HYDROGEN (CODE REVIEW, SHADER DEBUGGING & CONSULTING)", 1.2f, muted, 60.0f);

    // =========================================================================
    // 6. TEXTURES (ИЗ CREDITS.MD)
    // =========================================================================
    add("TEXTURES & SPRITES", 2.05f, red, 22.0f);
    add("SOURCED FROM OPENGAMEART.ORG, ITCH.IO, FREEDOOM & GITHUB", 1.25f, muted, 8.0f);
    add("LICENSES: CC0 1.0 (PUBLIC DOMAIN), CC-BY 4.0 & BSD-3-CLAUSE", 1.25f, muted, 22.0f);

    add("TEXTURES/LVL2_TEX/  -  RETRO BRICK, STONE WALLS & EXIT DOOR", 1.25f, white, 12.0f);
    add("TEXTURES/LVL3_TEX/  -  INDUSTRIAL CONCRETE, TILES & BILLBOARDS", 1.25f, white, 12.0f);
    add("TEXTURES/LVL4_TEX/  -  OFFICE WALLPAPER, CARPET, CEILING & EXIT", 1.25f, white, 60.0f);

    // =========================================================================
    // 7. AUDIO ASSETS (ИЗ CREDITS.MD)
    // =========================================================================
    add("AUDIO ASSETS", 2.05f, red, 22.0f);
    add("SOURCED FROM FREESOUND.ORG, OPENGAMEART.ORG, ITCH.IO & FREEDOOM", 1.25f, muted, 8.0f);
    add("LICENSES: CC0 1.0 (PUBLIC DOMAIN), CC-BY 4.0 & BSD-3-CLAUSE", 1.25f, muted, 22.0f);

    add("SOUNDS/LEVEL2/ & LEVEL3/  -  SYNTH/INDUSTRIAL LOOPS & FOOTSTEPS", 1.25f, white, 12.0f);
    add("SOUNDS/LEVEL4/  -  STEPS, FLASHLIGHT CLICKS, LAMP BUZZ & TRIGGERS", 1.25f, white, 72.0f);

    // =========================================================================
    // 8. ФИНАЛ
    // =========================================================================
    add("THANK YOU FOR VISITING THE MUSEUM!", 2.2f, gold, 24.0f);
    add("PRESS [ENTER] OR [ESC] TO RETURN TO MAIN MENU", 1.4f, muted, 40.0f);

    totalHeight = 0.0f;
    for (const auto& l : lines) {
        totalHeight += 8.0f * l.scale + l.spacingAfter;
    }

    int winW = 1280, winH = 720;
    glfwGetWindowSize(glfwGetCurrentContext(), &winW, &winH);
    scrollY = static_cast<float>(winH) + 30.0f;
    exitKeyReleased = false;
    fastForward = false;
}

void CreditsLevel::processInput(GLFWwindow* window, float deltaTime) {
    (void)deltaTime;

    fastForward = (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) ||
                  (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS);

    bool exitDown = (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) ||
                    (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS);

    if (!exitDown) {
        exitKeyReleased = true;
    } else if (exitKeyReleased) {
        gameEngine->changeLevel(new MainMenuLevel(gameEngine));
    }
}

void CreditsLevel::update(float deltaTime) {
    float speed = fastForward ? 190.0f : 52.0f;
    scrollY -= speed * deltaTime;

    if (scrollY + totalHeight < -40.0f) {
        gameEngine->changeLevel(new MainMenuLevel(gameEngine));
    }
}

void CreditsLevel::render() {
    int winW = 1280, winH = 720;
    glfwGetWindowSize(glfwGetCurrentContext(), &winW, &winH);
    if (winW <= 0 || winH <= 0) return;

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glClearColor(0.025f, 0.025f, 0.035f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    float w = static_cast<float>(winW);
    float h = static_cast<float>(winH);
    glm::mat4 proj = glm::ortho(0.0f, w, h, 0.0f);

    HudOverlay& hud = gameEngine->getHud();

    float curY = scrollY;
    for (const auto& line : lines) {
        float lineH = 8.0f * line.scale;
        if (curY + lineH >= -20.0f && curY <= h + 20.0f) {
            float lineW = line.text.size() * 8.0f * line.scale;
            hud.drawText(line.text, (w - lineW) * 0.5f, curY, line.scale, line.color, proj);
        }
        curY += lineH + line.spacingAfter;
    }

    std::string hint = "[HOLD SPACE] FAST FORWARD    [ESC / ENTER] MAIN MENU";
    float hintW = hint.size() * 8.0f * 1.25f;
    hud.drawText(hint, (w - hintW) * 0.5f, h - 32.0f, 1.25f,
                 glm::vec4(0.55f, 0.58f, 0.65f, 0.85f), proj);
}