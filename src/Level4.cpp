#include "Levels/Level4.hpp"
#include "Engine/stb_image.h"
#include "Levels/MainMenuLevel.hpp"
#include "Levels/CreditsLevel.hpp"
#include <iostream>
#include <random>
#include <algorithm>
#include <queue>
#include <cmath>
#include <cstdio>
#include <filesystem>

Level4::Level4(Engine* engine)
    : gameEngine(engine), shader(nullptr), spriteShader(nullptr),
      compassShader(nullptr), ps1PostShader(nullptr),
      maze(nullptr), audio(nullptr),
      compassVAO(0), compassVBO(0), pdaQuadVAO(0), pdaQuadVBO(0),
      ps1FBO(0), ps1ColorTex(0), ps1DepthRBO(0), screenQuadVAO(0), screenQuadVBO(0),
      floorTexture(0), ceilTexture(0), monsterTex1(0), monsterTex2(0),
      monsterSprite(nullptr),
      breakerPos(1.5f, 0.52f, 1.5f), breakerYaw(0.0f),
      powerRestored(false), leverAngle(-35.0f),
      stamina(100.0f), maxStamina(100.0f), isSprinting(false), isExhausted(false),
      catchFlashTimer(0.0f),
      flashlightOn(true), fKeyPressed(false), eKeyPressed(false),
      nKeyPressed(false), noclip(false),
      firstMouse(true), lastX(640.0f), lastY(360.0f), elapsedTime(0.0f),
      pdaHeld(false), pdaOpenProgress(0.0f) {}

Level4::~Level4() {
    if (shader) delete shader;
    if (spriteShader) delete spriteShader;
    if (compassShader) delete compassShader;
    if (ps1PostShader) delete ps1PostShader;
    if (maze) delete maze;
    if (audio) delete audio;
    if (monsterSprite) delete monsterSprite;

    for (int i = 0; i < 4; ++i) wallBatches[i].destroy();
    floorCeilMesh.destroy();
    lampPanelMesh.destroy();
    fanBaseMesh.destroy();
    fanBladeMesh.destroy();
    switchBoxMesh.destroy();
    switchLeverMesh.destroy();

    if (compassVAO) glDeleteVertexArrays(1, &compassVAO);
    if (compassVBO) glDeleteBuffers(1, &compassVBO);
    if (pdaQuadVAO) glDeleteVertexArrays(1, &pdaQuadVAO);
    if (pdaQuadVBO) glDeleteBuffers(1, &pdaQuadVBO);
    if (screenQuadVAO) glDeleteVertexArrays(1, &screenQuadVAO);
    if (screenQuadVBO) glDeleteBuffers(1, &screenQuadVBO);

    if (ps1FBO) glDeleteFramebuffers(1, &ps1FBO);
    if (ps1ColorTex) glDeleteTextures(1, &ps1ColorTex);
    if (ps1DepthRBO) glDeleteRenderbuffers(1, &ps1DepthRBO);

    for (GLuint tex : wallTextures) {
        if (tex != 0) glDeleteTextures(1, &tex);
    }
    if (floorTexture != 0) glDeleteTextures(1, &floorTexture);
    if (ceilTexture != 0) glDeleteTextures(1, &ceilTexture);
    if (monsterTex1 != 0) glDeleteTextures(1, &monsterTex1);
    if (monsterTex2 != 0) glDeleteTextures(1, &monsterTex2);

    Sprite::CleanupSharedMesh();
}

void Level4::playSoundSafe(const std::string& path) {
    if (audio && std::filesystem::exists(path)) {
        audio->playSound(path.c_str());
    }
}

GLuint Level4::createFallbackMonsterTexture() {
    const int w = 32;
    const int h = 64;
    std::vector<unsigned char> pixels(w * h * 4, 0);

    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            float nx = (x - w * 0.5f) / (w * 0.5f);
            float ny = static_cast<float>(y) / static_cast<float>(h);
            int idx = (y * w + x) * 4;

            // Вытянутый темный силуэт с рваным низом
            float widthLimit = (ny > 0.78f) ? 0.35f : (0.55f - 0.15f * std::sin(ny * 10.0f));
            if (std::abs(nx) < widthLimit && ny > (0.06f + 0.05f * std::sin(x * 1.7f))) {
                pixels[idx + 0] = 18;
                pixels[idx + 1] = 15;
                pixels[idx + 2] = 20;
                pixels[idx + 3] = 240;
            }

            // Два светящихся красных глаза
            if (ny > 0.80f && ny < 0.85f &&
                ((x >= 10 && x <= 12) || (x >= 19 && x <= 21))) {
                pixels[idx + 0] = 255;
                pixels[idx + 1] = 35;
                pixels[idx + 2] = 35;
                pixels[idx + 3] = 255;
            }
        }
    }

    GLuint texID;
    glGenTextures(1, &texID);
    glBindTexture(GL_TEXTURE_2D, texID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
    return texID;
}

GLuint Level4::loadTexture(const char* path, bool clampToEdge, bool allowMissing) {
    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(true);
    unsigned char* data = stbi_load(path, &width, &height, &nrChannels, 0);

    if (!data && allowMissing) {
        return 0;
    }

    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    GLint wrapMode = clampToEdge ? GL_CLAMP_TO_EDGE : GL_REPEAT;
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapMode);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapMode);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    if (data) {
        GLenum format = (nrChannels == 4) ? GL_RGBA : (nrChannels == 1 ? GL_RED : GL_RGB);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        stbi_image_free(data);
    } else {
        std::cout << "Fallback texture for: " << path << std::endl;
        unsigned char fallback[4] = { 75, 78, 72, 255 };
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, fallback);
    }

    return textureID;
}

void Level4::initPs1Framebuffer() {
    glGenFramebuffers(1, &ps1FBO);
    glBindFramebuffer(GL_FRAMEBUFFER, ps1FBO);

    glGenTextures(1, &ps1ColorTex);
    glBindTexture(GL_TEXTURE_2D, ps1ColorTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, PS1_WIDTH, PS1_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ps1ColorTex, 0);

    glGenRenderbuffers(1, &ps1DepthRBO);
    glBindRenderbuffer(GL_RENDERBUFFER, ps1DepthRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, PS1_WIDTH, PS1_HEIGHT);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, ps1DepthRBO);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cout << "ERROR: PS1 Framebuffer is not complete!" << std::endl;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    float screenVerts[] = {
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
        -1.0f,  1.0f,  0.0f, 1.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f
    };

    glGenVertexArrays(1, &screenQuadVAO);
    glGenBuffers(1, &screenQuadVBO);
    glBindVertexArray(screenQuadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, screenQuadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(screenVerts), screenVerts, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);
}

void Level4::init() {
    camera = Camera(glm::vec3(1.5f, 0.5f, 1.5f));
    camera.AllowPitch = true;
    camera.SetPose(glm::vec3(1.5f, 0.5f, 1.5f), -90.0f, 0.0f);

    shader = new Shader("shaders/level4.vert", "shaders/level4.frag");
    spriteShader = new Shader("shaders/sprite.vert", "shaders/sprite.frag");
    compassShader = new Shader("shaders/compass.vert", "shaders/compass.frag");
    ps1PostShader = new Shader("shaders/ps1_post.vert", "shaders/ps1_post.frag");

    initPs1Framebuffer();

    const int mazeSize = 25;
    maze = new MazeGenerator(mazeSize, mazeSize);
    maze->generate();

    PrimitiveMesh::BuildLitMaze(maze->getGrid(), wallBatches);

    wallTextures.push_back(loadTexture("textures/lvl4_tex/lvl4_wall_1.png"));
    wallTextures.push_back(loadTexture("textures/lvl4_tex/lvl4_wall_1.png"));
    wallTextures.push_back(loadTexture("textures/lvl4_tex/lvl4_wall_2.png"));
    wallTextures.push_back(loadTexture("textures/lvl4_tex/lvl4_exit.png"));

    floorTexture = loadTexture("textures/lvl4_tex/lvl4_floor.png");
    ceilTexture  = loadTexture("textures/lvl4_tex/lvl4_ceil.png");

    // Текстуры монстра (с авто-фоллбэком на спрайт манекена из Level3 или процедурный силуэт)
    monsterTex1 = loadTexture("textures/lvl4_tex/lvl4_monster.png", true, true);
    if (monsterTex1 == 0) {
        monsterTex1 = loadTexture("textures/lvl3_tex/lvl3_sprite_2.png", true, true);
    }
    if (monsterTex1 == 0) {
        monsterTex1 = createFallbackMonsterTexture();
    }

    monsterTex2 = loadTexture("textures/lvl4_tex/lvl4_monster_2.png", true, true);
    if (monsterTex2 == 0) {
        monsterTex2 = monsterTex1;
    }

    float s = static_cast<float>(mazeSize);
    std::vector<float> fcVerts = {
        0.0f, 0.0f, 0.0f,  0.0f, 0.0f,  0.0f,  1.0f, 0.0f,
           s, 0.0f, 0.0f,     s, 0.0f,  0.0f,  1.0f, 0.0f,
           s, 0.0f,    s,     s,    s,  0.0f,  1.0f, 0.0f,
        0.0f, 0.0f, 0.0f,  0.0f, 0.0f,  0.0f,  1.0f, 0.0f,
           s, 0.0f,    s,     s,    s,  0.0f,  1.0f, 0.0f,
        0.0f, 0.0f,    s,  0.0f,    s,  0.0f,  1.0f, 0.0f,
        0.0f, 1.0f, 0.0f,  0.0f, 0.0f,  0.0f, -1.0f, 0.0f,
           s, 1.0f,    s,     s,    s,  0.0f, -1.0f, 0.0f,
           s, 1.0f, 0.0f,     s, 0.0f,  0.0f, -1.0f, 0.0f,
        0.0f, 1.0f, 0.0f,  0.0f, 0.0f,  0.0f, -1.0f, 0.0f,
        0.0f, 1.0f,    s,  0.0f,    s,  0.0f, -1.0f, 0.0f,
           s, 1.0f,    s,     s,    s,  0.0f, -1.0f, 0.0f
    };

    glGenVertexArrays(1, &floorCeilMesh.VAO);
    glGenBuffers(1, &floorCeilMesh.VBO);
    floorCeilMesh.vertexCount = 12;
    glBindVertexArray(floorCeilMesh.VAO);
    glBindBuffer(GL_ARRAY_BUFFER, floorCeilMesh.VBO);
    glBufferData(GL_ARRAY_BUFFER, fcVerts.size() * sizeof(float), fcVerts.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(5 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glBindVertexArray(0);

    lampPanelMesh   = PrimitiveMesh::CreateBox(0.46f, 0.015f, 0.46f);
    fanBaseMesh     = PrimitiveMesh::CreateBox(0.14f, 0.08f, 0.14f);
    fanBladeMesh    = PrimitiveMesh::CreateBox(0.56f, 0.015f, 0.07f);
    switchBoxMesh   = PrimitiveMesh::CreateBox(0.24f, 0.34f, 0.10f);
    switchLeverMesh = PrimitiveMesh::CreateBox(0.04f, 0.18f, 0.04f);

    // Треугольник игрока для КПК
    float compassVertices[] = { 0.0f, 8.0f, -5.5f, -6.0f, 5.5f, -6.0f };
    glGenVertexArrays(1, &compassVAO);
    glGenBuffers(1, &compassVBO);
    glBindVertexArray(compassVAO);
    glBindBuffer(GL_ARRAY_BUFFER, compassVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(compassVertices), compassVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);

    // Единичный квадрат [0..1] для отрисовки элементов КПК
    float pdaQuadVerts[] = {
        0.0f, 0.0f,  1.0f, 0.0f,  1.0f, 1.0f,
        0.0f, 0.0f,  1.0f, 1.0f,  0.0f, 1.0f
    };
    glGenVertexArrays(1, &pdaQuadVAO);
    glGenBuffers(1, &pdaQuadVBO);
    glBindVertexArray(pdaQuadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, pdaQuadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(pdaQuadVerts), pdaQuadVerts, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);

    setupLampsAndTriggers();
    spawnMonster();

    audio = new Audio(1);
}

void Level4::spawnMonster() {
    const auto& grid = maze->getGrid();
    int rows = static_cast<int>(grid.size());
    int cols = static_cast<int>(grid[0].size());
    glm::vec2 playerPos(camera.Position.x, camera.Position.z);

    std::vector<glm::vec2> candidates;
    for (int z = 1; z < rows - 1; ++z) {
        for (int x = 1; x < cols - 1; ++x) {
            if (grid[z][x] == 0) {
                glm::vec2 p(x + 0.5f, z + 0.5f);
                float d = glm::distance(p, playerPos);
                if (d >= 6.5f && d <= 13.0f) {
                    candidates.push_back(p);
                }
            }
        }
    }

    if (candidates.empty()) {
        for (int z = 1; z < rows - 1; ++z) {
            for (int x = 1; x < cols - 1; ++x) {
                if (grid[z][x] == 0) {
                    glm::vec2 p(x + 0.5f, z + 0.5f);
                    if (glm::distance(p, playerPos) >= 4.0f) {
                        candidates.push_back(p);
                    }
                }
            }
        }
    }

    glm::vec2 spawnPt(rows - 2.5f, cols - 2.5f);
    if (!candidates.empty()) {
        std::mt19937 gen(std::random_device{}());
        std::uniform_int_distribution<size_t> dist(0, candidates.size() - 1);
        spawnPt = candidates[dist(gen)];
    }

    monster.pos = glm::vec3(spawnPt.x, 0.0f, spawnPt.y);
    monster.targetPos = monster.pos;
    monster.hasTarget = false;
    monster.state = MonsterState::PATROL;
    monster.patrolSpeed = 1.45f;
    monster.chaseSpeed = 2.65f;
    monster.animTimer = 0.0f;
    monster.animFrame = 0;
    monster.groanTimer = 3.0f;

    if (monsterSprite) delete monsterSprite;
    monsterSprite = new Sprite(monster.pos, glm::vec2(0.68f, 0.92f), monsterTex1);
}

bool Level4::hasLineOfSight(const glm::vec3& from, const glm::vec3& to) const {
    const auto& grid = maze->getGrid();
    int rows = static_cast<int>(grid.size());
    int cols = static_cast<int>(grid[0].size());

    glm::vec2 a(from.x, from.z);
    glm::vec2 b(to.x, to.z);
    float dist = glm::distance(a, b);
    int steps = static_cast<int>(dist * 5.0f);
    if (steps <= 1) return true;

    for (int i = 1; i < steps; ++i) {
        float t = static_cast<float>(i) / static_cast<float>(steps);
        glm::vec2 p = glm::mix(a, b, t);
        int gx = static_cast<int>(p.x);
        int gz = static_cast<int>(p.y);
        if (gz < 0 || gz >= rows || gx < 0 || gx >= cols) return false;
        if (grid[gz][gx] == 1) return false;
    }
    return true;
}

glm::ivec2 Level4::getNextStepBFS(glm::ivec2 start, glm::ivec2 goal) const {
    const auto& grid = maze->getGrid();
    int rows = static_cast<int>(grid.size());
    int cols = static_cast<int>(grid[0].size());

    if (start == goal) return start;
    if (goal.y < 0 || goal.y >= rows || goal.x < 0 || goal.x >= cols) return start;

    std::vector<std::vector<glm::ivec2>> parent(rows, std::vector<glm::ivec2>(cols, glm::ivec2(-1, -1)));
    std::queue<glm::ivec2> q;

    parent[start.y][start.x] = start;
    q.push(start);

    const glm::ivec2 dirs[4] = { {1, 0}, {-1, 0}, {0, 1}, {0, -1} };
    bool found = false;

    while (!q.empty()) {
        glm::ivec2 cur = q.front();
        q.pop();

        if (cur == goal) {
            found = true;
            break;
        }

        for (const auto& d : dirs) {
            glm::ivec2 nxt = cur + d;
            if (nxt.y >= 0 && nxt.y < rows && nxt.x >= 0 && nxt.x < cols) {
                if (parent[nxt.y][nxt.x].x == -1 && grid[nxt.y][nxt.x] != 1) {
                    parent[nxt.y][nxt.x] = cur;
                    q.push(nxt);
                }
            }
        }
    }

    if (!found) return start;

    glm::ivec2 step = goal;
    while (parent[step.y][step.x] != start && parent[step.y][step.x].x != -1) {
        step = parent[step.y][step.x];
    }
    return step;
}

void Level4::updateMonsterAI(float deltaTime) {
    const auto& grid = maze->getGrid();
    int rows = static_cast<int>(grid.size());
    int cols = static_cast<int>(grid[0].size());

    glm::vec2 mPos2D(monster.pos.x, monster.pos.z);
    glm::vec2 pPos2D(camera.Position.x, camera.Position.z);
    float distToPlayer = glm::distance(mPos2D, pPos2D);

    // Анимация билборда монстра
    monster.animTimer += deltaTime * (monster.state == MonsterState::CHASE ? 1.8f : 1.0f);
    if (monster.animTimer >= 0.32f) {
        monster.animTimer = 0.0f;
        monster.animFrame = 1 - monster.animFrame;
        if (monsterSprite) {
            monsterSprite->TextureID = (monster.animFrame == 0) ? monsterTex1 : monsterTex2;
        }
    }

    // Проверка обнаружения игрока (фонарик увеличивает заметность, бег создает шум)
    float sightRange = flashlightOn ? 9.5f : 5.5f;
    bool canSee = !noclip && (distToPlayer <= sightRange) && hasLineOfSight(monster.pos, camera.Position);
    bool canHearSprint = !noclip && isSprinting && (distToPlayer <= 4.5f);

    if (canSee || canHearSprint) {
        if (monster.state != MonsterState::CHASE) {
            playSoundSafe("sounds/Level4/Monster_Alert.wav");
            if (!std::filesystem::exists("sounds/Level4/Monster_Alert.wav")) {
                playSoundSafe("sounds/Level4/Trigger_1.wav");
            }
        }
        monster.state = MonsterState::CHASE;
        monster.loseSightTimer = 4.0f;
    }

    // Периодические звуки присутствия рядом с игроком
    monster.groanTimer -= deltaTime;
    if (monster.groanTimer <= 0.0f && distToPlayer < 9.0f) {
        playSoundSafe("sounds/Level4/Monster_Groan.wav");
        monster.groanTimer = (monster.state == MonsterState::CHASE) ? 3.2f : 6.5f;
    }

    // Выбор следующей клетки маршрута
    if (!monster.hasTarget || glm::distance(mPos2D, glm::vec2(monster.targetPos.x, monster.targetPos.z)) < 0.08f) {
        int mx = std::clamp(static_cast<int>(monster.pos.x), 1, cols - 2);
        int mz = std::clamp(static_cast<int>(monster.pos.z), 1, rows - 2);
        int px = std::clamp(static_cast<int>(camera.Position.x), 1, cols - 2);
        int pz = std::clamp(static_cast<int>(camera.Position.z), 1, rows - 2);

        glm::ivec2 chosen(mx, mz);

        if (monster.state == MonsterState::CHASE) {
            chosen = getNextStepBFS(glm::ivec2(mx, mz), glm::ivec2(px, pz));
        } else {
            const glm::ivec2 dirs[4] = { {1, 0}, {-1, 0}, {0, 1}, {0, -1} };
            std::vector<glm::ivec2> validNeighbors;
            for (const auto& d : dirs) {
                int nx = mx + d.x;
                int nz = mz + d.y;
                if (nz >= 1 && nz < rows - 1 && nx >= 1 && nx < cols - 1 && grid[nz][nx] == 0) {
                    validNeighbors.emplace_back(nx, nz);
                }
            }

            if (!validNeighbors.empty()) {
                static std::mt19937 aiGen(std::random_device{}());
                // Если монстр ушел слишком далеко от игрока, подтягиваем его поближе
                if (distToPlayer > 8.5f && (aiGen() % 100) < 75) {
                    std::sort(validNeighbors.begin(), validNeighbors.end(), [&](const glm::ivec2& a, const glm::ivec2& b) {
                        float da = glm::distance(glm::vec2(a.x + 0.5f, a.y + 0.5f), pPos2D);
                        float db = glm::distance(glm::vec2(b.x + 0.5f, b.y + 0.5f), pPos2D);
                        return da < db;
                    });
                    chosen = validNeighbors.front();
                } else {
                    std::uniform_int_distribution<size_t> pick(0, validNeighbors.size() - 1);
                    chosen = validNeighbors[pick(aiGen)];
                }
            }
        }

        monster.targetPos = glm::vec3(chosen.x + 0.5f, 0.0f, chosen.y + 0.5f);
        monster.hasTarget = true;
    }

    // Движение монстра к текущей путевой точке
    glm::vec2 target2D(monster.targetPos.x, monster.targetPos.z);
    glm::vec2 dir = target2D - mPos2D;
    float len = glm::length(dir);
    if (len > 0.001f) {
        float spd = (monster.state == MonsterState::CHASE) ? monster.chaseSpeed : monster.patrolSpeed;
        glm::vec2 step = (dir / len) * std::min(len, spd * deltaTime);
        monster.pos.x += step.x;
        monster.pos.z += step.y;
    }

    if (monsterSprite) {
        // Легкое покачивание при ходьбе
        float bob = std::abs(std::sin(elapsedTime * (monster.state == MonsterState::CHASE ? 11.0f : 6.0f))) * 0.03f;
        monsterSprite->Position = glm::vec3(monster.pos.x, bob, monster.pos.z);
    }

    // Проверка поимки игрока
    if (!noclip && glm::distance(glm::vec2(monster.pos.x, monster.pos.z), pPos2D) < 0.55f) {
        playSoundSafe("sounds/Level4/Monster_Catch.wav");
        if (!std::filesystem::exists("sounds/Level4/Monster_Catch.wav")) {
            playSoundSafe("sounds/Level4/Trigger_2.wav");
        }
        std::cout << "CAUGHT BY THE STALKER! Respawning at entrance..." << std::endl;
        camera.SetPose(glm::vec3(1.5f, 0.5f, 1.5f), -90.0f, 0.0f);
        stamina = maxStamina;
        isExhausted = false;
        catchFlashTimer = 1.2f;
        spawnMonster();
    }
}

void Level4::setupLampsAndTriggers() {
    officeLamps.clear();
    soundTriggers.clear();
    fanPositions.clear();

    const auto& grid = maze->getGrid();
    int rows = static_cast<int>(grid.size());
    int cols = static_cast<int>(grid[0].size());
    glm::vec2 startPos(1.5f, 1.5f);
    glm::vec2 exitPos(maze->getExitX() + 0.5f, maze->getExitY() + 0.5f);

    std::vector<glm::vec2> openCells;
    for (int z = 1; z < rows - 1; ++z) {
        for (int x = 1; x < cols - 1; ++x) {
            if (grid[z][x] == 0) {
                glm::vec2 p(x + 0.5f, z + 0.5f);
                if (glm::distance(p, startPos) > 2.2f) {
                    openCells.push_back(p);
                }
            }
        }
    }

    std::mt19937 gen(std::random_device{}());
    std::shuffle(openCells.begin(), openCells.end(), gen);

    for (const auto& cell : openCells) {
        if (officeLamps.size() >= 4) break;
        bool farEnough = true;
        for (const auto& l : officeLamps) {
            if (glm::distance(cell, glm::vec2(l.position.x, l.position.z)) < 5.5f) {
                farEnough = false;
                break;
            }
        }
        if (farEnough) {
            officeLamps.push_back({ glm::vec3(cell.x, 0.94f, cell.y), 0.0f, 0.25f, 1.0f, false });
        }
    }

    for (size_t i = 4; i < openCells.size() && fanPositions.size() < 3; ++i) {
        fanPositions.emplace_back(openCells[i].x, 0.96f, openCells[i].y);
    }

    struct WallMount {
        glm::vec3 pos;
        float yaw;
        float distToExit;
    };

    std::vector<WallMount> wallMounts;
    for (int z = 1; z < rows - 1; ++z) {
        for (int x = 1; x < cols - 1; ++x) {
            if (grid[z][x] != 0) continue;
            float cx = static_cast<float>(x) + 0.5f;
            float cz = static_cast<float>(z) + 0.5f;
            float dist = glm::distance(glm::vec2(cx, cz), exitPos);

            if (grid[z - 1][x] == 1) wallMounts.push_back({ glm::vec3(cx, 0.52f, static_cast<float>(z) + 0.05f), 0.0f, dist });
            if (grid[z + 1][x] == 1) wallMounts.push_back({ glm::vec3(cx, 0.52f, static_cast<float>(z + 1) - 0.05f), 180.0f, dist });
            if (grid[z][x - 1] == 1) wallMounts.push_back({ glm::vec3(static_cast<float>(x) + 0.05f, 0.52f, cz), 90.0f, dist });
            if (grid[z][x + 1] == 1) wallMounts.push_back({ glm::vec3(static_cast<float>(x + 1) - 0.05f, 0.52f, cz), -90.0f, dist });
        }
    }

    std::sort(wallMounts.begin(), wallMounts.end(), [](const WallMount& a, const WallMount& b) {
        return a.distToExit < b.distToExit;
    });

    if (!wallMounts.empty()) {
        breakerPos = wallMounts.front().pos;
        breakerYaw = wallMounts.front().yaw;
        for (const auto& wm : wallMounts) {
            if (wm.distToExit >= 1.5f && wm.distToExit <= 8.0f) {
                breakerPos = wm.pos;
                breakerYaw = wm.yaw;
                break;
            }
        }
    }

    const char* triggerSounds[3] = {
        "sounds/Level4/Trigger_1.wav",
        "sounds/Level4/Trigger_2.wav",
        "sounds/Level4/Trigger_3.wav"
    };
    int soundIdx = 0;
    for (size_t i = 7; i < openCells.size() && soundTriggers.size() < 8; i += 3) {
        soundTriggers.push_back({
            glm::vec3(openCells[i].x, 0.5f, openCells[i].y),
            1.65f,
            triggerSounds[soundIdx % 3],
            false
        });
        soundIdx++;
    }
}

std::string Level4::getHudHint() const {
    char buf[96];
    const char* objText = powerRestored ? "EXIT UNLOCKED!" : "BREAKER [E]";
    const char* stateText = (monster.state == MonsterState::CHASE) ? "RUN!" : (isExhausted ? "TIRED" : "OK");
    std::snprintf(buf, sizeof(buf), "%s | STAMINA: %.0f%% [%s]", objText, stamina, stateText);
    return std::string(buf);
}

bool Level4::checkCollision(float x, float z) {
    float radius = 0.2f;
    const auto& grid = maze->getGrid();

    int minX = static_cast<int>(x - radius);
    int maxX = static_cast<int>(x + radius);
    int minZ = static_cast<int>(z - radius);
    int maxZ = static_cast<int>(z + radius);

    if (minX < 0 || maxX >= static_cast<int>(grid[0].size()) ||
        minZ < 0 || maxZ >= static_cast<int>(grid.size())) return true;

    auto isBlocked = [&](int gx, int gz) {
        int cell = grid[gz][gx];
        if (cell == 1) return true;
        if (cell == 2 && !powerRestored) return true;
        return false;
    };

    return isBlocked(minX, minZ) || isBlocked(maxX, minZ) ||
           isBlocked(minX, maxZ) || isBlocked(maxX, maxZ);
}

void Level4::processInput(GLFWwindow* window, float deltaTime) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        gameEngine->changeLevel(new MainMenuLevel(gameEngine));
        return;
    }

    if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS) {
        if (!fKeyPressed) {
            flashlightOn = !flashlightOn;
            fKeyPressed = true;
            playSoundSafe("sounds/Level4/Flashlight.wav");
        }
    } else {
        fKeyPressed = false;
    }

    bool tabDown = (glfwGetKey(window, GLFW_KEY_TAB) == GLFW_PRESS);
    if (tabDown && !pdaHeld) {
        playSoundSafe("sounds/Level4/PDA_Open.wav");
    }
    pdaHeld = tabDown;

    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
        if (!eKeyPressed) {
            eKeyPressed = true;
            if (!powerRestored && glm::distance(camera.Position, breakerPos) < 1.8f) {
                powerRestored = true;
                playSoundSafe("sounds/Level4/Flashlight.wav");
                std::cout << "Power restored! Exit unlocked." << std::endl;
            }
        }
    } else {
        eKeyPressed = false;
    }

    if (glfwGetKey(window, GLFW_KEY_N) == GLFW_PRESS) {
        if (!nKeyPressed) {
            noclip = !noclip;
            nKeyPressed = true;
        }
    } else {
        nKeyPressed = false;
    }

    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
        camera.Jump();
    }

    double xposIn, yposIn;
    glfwGetCursorPos(window, &xposIn, &yposIn);
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    camera.ProcessMouseMovement(xpos - lastX, lastY - ypos);
    lastX = xpos;
    lastY = ypos;

    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)  camera.ProcessMouseMovement(-150.0f * deltaTime, 0.0f);
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) camera.ProcessMouseMovement(150.0f * deltaTime, 0.0f);
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)    camera.ProcessMouseMovement(0.0f, 120.0f * deltaTime);
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)  camera.ProcessMouseMovement(0.0f, -120.0f * deltaTime);

    glm::vec3 moveDir(0.0f);
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) moveDir += camera.Front;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) moveDir -= camera.Front;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) moveDir -= camera.Right;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) moveDir += camera.Right;

    bool wantsSprint = (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) && (glm::length(moveDir) > 0.0f);

    if (wantsSprint && !isExhausted && stamina > 0.0f) {
        isSprinting = true;
        stamina -= 34.0f * deltaTime;
        if (stamina <= 0.0f) {
            stamina = 0.0f;
            isSprinting = false;
            isExhausted = true;
        }
    } else {
        isSprinting = false;
        float regenRate = (glm::length(moveDir) > 0.0f) ? 14.0f : 24.0f;
        stamina = std::min(maxStamina, stamina + regenRate * deltaTime);
        if (isExhausted && stamina >= 25.0f) {
            isExhausted = false;
        }
    }

    float speedMult = noclip ? 2.2f : (isSprinting ? 1.62f : 1.0f);
    float velocity = camera.MovementSpeed * deltaTime * speedMult;

    static float stepTimer = 0.45f;
    float stepInterval = isSprinting ? 0.27f : 0.45f;

    if (glm::length(moveDir) > 0.0f) {
        if (noclip) {
            camera.Position += glm::normalize(moveDir) * velocity;
        } else {
            if (camera.IsGrounded) {
                stepTimer += deltaTime;
                if (stepTimer > stepInterval) {
                    playSoundSafe("sounds/Level4/Step.wav");
                    stepTimer = 0.0f;
                }
            }
            moveDir.y = 0.0f;
            if (glm::length(moveDir) > 0.0001f) {
                moveDir = glm::normalize(moveDir) * velocity;
                if (!checkCollision(camera.Position.x + moveDir.x, camera.Position.z)) camera.Position.x += moveDir.x;
                if (!checkCollision(camera.Position.x, camera.Position.z + moveDir.z)) camera.Position.z += moveDir.z;
            }
        }
    } else {
        stepTimer = stepInterval;
    }
}

void Level4::update(float deltaTime) {
    elapsedTime += deltaTime;
    if (catchFlashTimer > 0.0f) {
        catchFlashTimer = std::max(0.0f, catchFlashTimer - deltaTime);
    }

    if (!noclip) {
        camera.UpdatePhysics(deltaTime);
    }

    updateMonsterAI(deltaTime);

    float targetAngle = powerRestored ? 40.0f : -35.0f;
    leverAngle += (targetAngle - leverAngle) * std::min(1.0f, deltaTime * 8.0f);

    static std::mt19937 flickerGen(std::random_device{}());
    std::uniform_real_distribution<float> timeDist(0.05f, 0.35f);
    std::uniform_int_distribution<int> stateDist(0, 99);

    for (auto& lamp : officeLamps) {
        lamp.timer += deltaTime;
        if (lamp.timer >= lamp.nextFlicker) {
            lamp.timer = 0.0f;
            lamp.nextFlicker = timeDist(flickerGen);
            int roll = stateDist(flickerGen);
            if (roll < 35) {
                lamp.intensity = 0.0f;
                if (!lamp.soundPlayed && glm::distance(camera.Position, lamp.position) < 6.0f) {
                    playSoundSafe("sounds/Level4/Flicker.wav");
                    lamp.soundPlayed = true;
                }
            } else if (roll < 55) {
                lamp.intensity = 0.35f;
            } else {
                lamp.intensity = 1.0f;
                lamp.soundPlayed = false;
            }
        }
    }

    float targetPda = pdaHeld ? 1.0f : 0.0f;
    pdaOpenProgress += (targetPda - pdaOpenProgress) * std::min(1.0f, deltaTime * 12.0f);

    for (auto& trig : soundTriggers) {
        if (!trig.triggered && glm::distance(camera.Position, trig.position) <= trig.radius) {
            trig.triggered = true;
            playSoundSafe(trig.soundPath);
        }
    }

    const auto& grid = maze->getGrid();
    int gx = static_cast<int>(camera.Position.x);
    int gz = static_cast<int>(camera.Position.z);

    glm::vec2 player2D(camera.Position.x, camera.Position.z);
    glm::vec2 exitCenter(maze->getExitX() + 0.5f, maze->getExitY() + 0.5f);

    bool inExitCell = (gz >= 0 && gz < static_cast<int>(grid.size()) &&
                       gx >= 0 && gx < static_cast<int>(grid[0].size()) &&
                       grid[gz][gx] == 2);
    bool nearExit = (glm::distance(player2D, exitCenter) < 0.85f);

    if (powerRestored && (inExitCell || nearExit)) {
        std::cout << "LEVEL 4 COMPLETE! Rolling credits..." << std::endl;
        gameEngine->changeLevel(new CreditsLevel(gameEngine));
        return;
    }
}

void Level4::drawPdaRect(float x, float y, float w, float h, const glm::vec4& color, const glm::mat4& proj) {
    glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(x, y, 0.0f));
    model = glm::scale(model, glm::vec3(w, h, 1.0f));

    compassShader->use();
    compassShader->setMat4("projection", proj);
    compassShader->setMat4("model", model);
    compassShader->setVec4("uColor", color);

    glBindVertexArray(pdaQuadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}

void Level4::renderPDA(int screenW, int screenH) {
    // Если КПК полностью опущен — ничего не рисуем
    if (pdaOpenProgress <= 0.01f) return;

    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glm::mat4 orthoProj = glm::ortho(0.0f, static_cast<float>(screenW), 0.0f, static_cast<float>(screenH));

    // Лёгкое затемнение фона, пока игрок смотрит в экран КПК
    drawPdaRect(0.0f, 0.0f, static_cast<float>(screenW), static_cast<float>(screenH),
                glm::vec4(0.0f, 0.0f, 0.0f, 0.35f * pdaOpenProgress), orthoProj);

    // Крупный корпус КПК по центру экрана, выезжающий снизу
    const float pdaW = 340.0f;
    const float pdaH = 380.0f;
    const float pdaX = (static_cast<float>(screenW) - pdaW) * 0.5f;

    float closedY = -pdaH - 20.0f;
    float openY   = (static_cast<float>(screenH) - pdaH) * 0.5f;
    float pdaY    = glm::mix(closedY, openY, pdaOpenProgress);

    bool isChasing = (monster.state == MonsterState::CHASE);
    float distToMonster = glm::distance(glm::vec2(camera.Position.x, camera.Position.z),
                                        glm::vec2(monster.pos.x, monster.pos.z));

    // 1. Корпус и рамка устройства
    glm::vec4 borderCol = isChasing
        ? glm::vec4(0.95f, 0.20f, 0.20f, 0.92f)
        : glm::vec4(0.22f, 0.78f, 0.42f, 0.85f);

    drawPdaRect(pdaX - 4.0f, pdaY - 4.0f, pdaW + 8.0f, pdaH + 8.0f, borderCol, orthoProj);
    drawPdaRect(pdaX, pdaY, pdaW, pdaH, glm::vec4(0.04f, 0.07f, 0.05f, 0.93f), orthoProj);

    // 2. Полоска стамины в нижней части КПК
    float barX = pdaX + 16.0f;
    float barY = pdaY + 16.0f;
    float barW = pdaW - 32.0f;
    float barH = 14.0f;

    drawPdaRect(barX, barY, barW, barH, glm::vec4(0.10f, 0.16f, 0.12f, 0.95f), orthoProj);
    float stamRatio = std::clamp(stamina / maxStamina, 0.0f, 1.0f);
    glm::vec4 stamColor = isExhausted
        ? glm::vec4(0.92f, 0.25f, 0.22f, 0.95f)
        : (stamRatio < 0.35f ? glm::vec4(0.95f, 0.75f, 0.20f, 0.95f) : glm::vec4(0.25f, 0.90f, 0.48f, 0.95f));
    drawPdaRect(barX + 2.0f, barY + 2.0f, (barW - 4.0f) * stamRatio, barH - 4.0f, stamColor, orthoProj);

    // 3. Экран локального радара (308x308 px)
    float mapX = pdaX + 16.0f;
    float mapY = pdaY + 42.0f;
    float mapSize = pdaW - 32.0f;
    float mapCenterX = mapX + mapSize * 0.5f;
    float mapCenterY = mapY + mapSize * 0.5f;

    drawPdaRect(mapX, mapY, mapSize, mapSize, glm::vec4(0.02f, 0.05f, 0.03f, 0.96f), orthoProj);

    glEnable(GL_SCISSOR_TEST);
    glScissor(static_cast<GLint>(mapX), static_cast<GLint>(mapY),
              static_cast<GLsizei>(mapSize), static_cast<GLsizei>(mapSize));

    const float radarRadiusCells = 6.5f;
    const float cellPx = (mapSize * 0.5f) / radarRadiusCells;

    float mapRot = glm::radians(camera.Yaw) + glm::half_pi<float>();
    float cosR = std::cos(mapRot);
    float sinR = std::sin(mapRot);

    // Векторы направления взгляда и правой стороны в горизонтальной плоскости (X, Z)
    glm::vec2 fwd2D(camera.Front.x, camera.Front.z);
    if (glm::length(fwd2D) > 0.0001f) fwd2D = glm::normalize(fwd2D);
    else fwd2D = glm::vec2(0.0f, -1.0f);

    glm::vec2 right2D(camera.Right.x, camera.Right.z);
    if (glm::length(right2D) > 0.0001f) right2D = glm::normalize(right2D);
    else right2D = glm::vec2(1.0f, 0.0f);

    auto worldToRadar = [&](float wx, float wz) -> glm::vec2 {
        glm::vec2 d(wx - camera.Position.x, wz - camera.Position.z);
        float rx = glm::dot(d, right2D); // смещение вправо от игрока
        float ry = glm::dot(d, fwd2D);   // смещение вперёд от игрока
        return glm::vec2(mapCenterX + rx * cellPx, mapCenterY + ry * cellPx);
    };

    // Сетка прицела и бегущая линия сканирования
    drawPdaRect(mapCenterX - 0.5f, mapY, 1.0f, mapSize, glm::vec4(0.15f, 0.40f, 0.22f, 0.35f), orthoProj);
    drawPdaRect(mapX, mapCenterY - 0.5f, mapSize, 1.0f, glm::vec4(0.15f, 0.40f, 0.22f, 0.35f), orthoProj);

    float sweepOffset = std::fmod(elapsedTime * 140.0f, mapSize);
    drawPdaRect(mapX, mapY + sweepOffset, mapSize, 2.0f, glm::vec4(0.25f, 0.90f, 0.45f, 0.22f), orthoProj);

    // Отрисовка стен и выхода в ограниченном радиусе вокруг игрока
    const auto& grid = maze->getGrid();
    int rows = static_cast<int>(grid.size());
    int cols = static_cast<int>(grid[0].size());
    int pCellX = static_cast<int>(camera.Position.x);
    int pCellZ = static_cast<int>(camera.Position.z);
    int scanRange = 8;

    compassShader->use();
    compassShader->setMat4("projection", orthoProj);
    glBindVertexArray(pdaQuadVAO);

    for (int gz = pCellZ - scanRange; gz <= pCellZ + scanRange; ++gz) {
        for (int gx = pCellX - scanRange; gx <= pCellX + scanRange; ++gx) {
            if (gz < 0 || gz >= rows || gx < 0 || gx >= cols) continue;

            int cell = grid[gz][gx];
            if (cell == 0) continue;

            glm::vec2 cellCenter(gx + 0.5f, gz + 0.5f);
            if (glm::distance(cellCenter, glm::vec2(camera.Position.x, camera.Position.z)) > radarRadiusCells + 0.8f) {
                continue;
            }

            glm::vec2 screenPos = worldToRadar(cellCenter.x, cellCenter.y);
            glm::vec4 cellColor = (cell == 2)
                ? (powerRestored ? glm::vec4(0.20f, 0.95f, 0.85f, 0.95f) : glm::vec4(0.85f, 0.30f, 0.25f, 0.85f))
                : glm::vec4(0.18f, 0.55f, 0.30f, 0.78f);

            float cellAngle = std::atan2(fwd2D.x, right2D.x);
            glm::mat4 mCell = glm::translate(glm::mat4(1.0f), glm::vec3(screenPos.x, screenPos.y, 0.0f));
            mCell = glm::rotate(mCell, cellAngle, glm::vec3(0.0f, 0.0f, 1.0f));
            mCell = glm::translate(mCell, glm::vec3(-cellPx * 0.48f, -cellPx * 0.48f, 0.0f));
            mCell = glm::scale(mCell, glm::vec3(cellPx * 0.96f, cellPx * 0.96f, 1.0f));

            compassShader->setMat4("model", mCell);
            compassShader->setVec4("uColor", cellColor);
            glDrawArrays(GL_TRIANGLES, 0, 6);
        }
    }

    // Жёлтая метка рубильника
    if (!powerRestored && glm::distance(glm::vec2(breakerPos.x, breakerPos.z),
                                        glm::vec2(camera.Position.x, camera.Position.z)) <= radarRadiusCells) {
        glm::vec2 bScreen = worldToRadar(breakerPos.x, breakerPos.z);
        drawPdaRect(bScreen.x - 5.0f, bScreen.y - 5.0f, 10.0f, 10.0f, glm::vec4(0.98f, 0.85f, 0.20f, 0.98f), orthoProj);
    }

    // Пульсирующая красная метка монстра
    if (distToMonster <= radarRadiusCells + 0.5f) {
        glm::vec2 mScreen = worldToRadar(monster.pos.x, monster.pos.z);
        float pulse = 0.55f + 0.45f * std::sin(elapsedTime * 12.0f);
        float blipSize = 11.0f + 4.0f * pulse;
        drawPdaRect(mScreen.x - blipSize * 0.5f, mScreen.y - blipSize * 0.5f,
                    blipSize, blipSize, glm::vec4(1.0f, 0.18f, 0.18f, 0.65f + 0.35f * pulse), orthoProj);
    }

    glDisable(GL_SCISSOR_TEST);

    // 4. Стрелка игрока в центре КПК
    compassShader->use();
    compassShader->setMat4("projection", orthoProj);
    glm::mat4 modelPlayer = glm::translate(glm::mat4(1.0f), glm::vec3(mapCenterX, mapCenterY, 0.0f));
    modelPlayer = glm::scale(modelPlayer, glm::vec3(1.25f, 1.25f, 1.0f));
    compassShader->setMat4("model", modelPlayer);
    compassShader->setVec4("uColor", glm::vec4(0.85f, 1.0f, 0.90f, 1.0f));

    glBindVertexArray(compassVAO);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glBindVertexArray(0);

    glEnable(GL_DEPTH_TEST);
}

void Level4::render() {
    int width, height;
    glfwGetFramebufferSize(glfwGetCurrentContext(), &width, &height);
    if (width <= 0 || height <= 0) return;

    // 1. Рендерим 3D-сцену в низкополигональный PS1-буфер 320x240
    glBindFramebuffer(GL_FRAMEBUFFER, ps1FBO);
    glViewport(0, 0, PS1_WIDTH, PS1_HEIGHT);

    // 2. Выводим PS1-буфер на основной экран с дизерингом и квантованием цвета
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, width, height);
    glDisable(GL_DEPTH_TEST);
    glClearColor(0.01f, 0.01f, 0.015f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glClearColor(0.01f, 0.01f, 0.015f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);

    glm::mat4 projection = glm::perspective(glm::radians(60.0f),
                                            static_cast<float>(PS1_WIDTH) / static_cast<float>(PS1_HEIGHT),
                                            0.1f, 100.0f);
    glm::mat4 view = camera.GetViewMatrix();

    shader->use();
    shader->setMat4("projection", projection);
    shader->setMat4("view", view);
    shader->setVec3("viewPos", camera.Position);
    shader->setInt("texDiffuse", 0);
    shader->setInt("useSolidColor", 0);
    shader->setFloat("emissiveStrength", 0.0f);

    shader->setVec3("flashlight.position", camera.Position);
    shader->setVec3("flashlight.direction", camera.Front);
    shader->setFloat("flashlight.cutOff", std::cos(glm::radians(13.5f)));
    shader->setFloat("flashlight.outerCutOff", std::cos(glm::radians(21.0f)));
    shader->setVec3("flashlight.ambient", glm::vec3(0.02f, 0.02f, 0.02f));
    shader->setVec3("flashlight.diffuse", glm::vec3(0.55f, 0.53f, 0.46f));
    shader->setVec3("flashlight.specular", glm::vec3(0.45f, 0.45f, 0.40f));
    shader->setFloat("flashlight.constant", 1.0f);
    shader->setFloat("flashlight.linear", 0.14f);
    shader->setFloat("flashlight.quadratic", 0.07f);
    shader->setInt("flashlight.enabled", flashlightOn ? 1 : 0);

    for (int i = 0; i < 4; ++i) {
        std::string base = "pointLights[" + std::to_string(i) + "]";
        if (i < static_cast<int>(officeLamps.size())) {
            shader->setVec3(base + ".position", officeLamps[i].position);
            shader->setVec3(base + ".ambient", glm::vec3(0.03f, 0.035f, 0.03f));
            shader->setVec3(base + ".diffuse", glm::vec3(0.85f, 0.90f, 0.82f));
            shader->setVec3(base + ".specular", glm::vec3(0.6f, 0.65f, 0.6f));
            shader->setFloat(base + ".constant", 1.0f);
            shader->setFloat(base + ".linear", 0.22f);
            shader->setFloat(base + ".quadratic", 0.20f);
            shader->setFloat(base + ".intensity", officeLamps[i].intensity);
        } else {
            shader->setFloat(base + ".intensity", 0.0f);
        }
    }

    glm::mat4 identity(1.0f);
    shader->setMat4("model", identity);
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(floorCeilMesh.VAO);
    glBindTexture(GL_TEXTURE_2D, floorTexture);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindTexture(GL_TEXTURE_2D, ceilTexture);
    glDrawArrays(GL_TRIANGLES, 6, 6);
    glBindVertexArray(0);

    for (int i = 0; i < 4; ++i) {
        if (wallBatches[i].vertexCount == 0) continue;
        glBindTexture(GL_TEXTURE_2D, wallTextures[i]);
        wallBatches[i].draw();
    }

    shader->setInt("useSolidColor", 1);
    for (const auto& lamp : officeLamps) {
        glm::mat4 mLamp = glm::translate(glm::mat4(1.0f), glm::vec3(lamp.position.x, 0.992f, lamp.position.z));
        shader->setMat4("model", mLamp);
        if (lamp.intensity > 0.1f) {
            shader->setVec3("solidColor", glm::vec3(0.92f, 0.96f, 0.90f));
            shader->setFloat("emissiveStrength", lamp.intensity);
        } else {
            shader->setVec3("solidColor", glm::vec3(0.22f, 0.23f, 0.24f));
            shader->setFloat("emissiveStrength", 0.0f);
        }
        lampPanelMesh.draw();
    }

    shader->setFloat("emissiveStrength", 0.0f);
    for (const auto& fPos : fanPositions) {
        glm::mat4 parentFan = glm::translate(glm::mat4(1.0f), fPos);
        shader->setMat4("model", parentFan);
        shader->setVec3("solidColor", glm::vec3(0.18f, 0.19f, 0.20f));
        fanBaseMesh.draw();

        glm::mat4 childBlade1 = glm::rotate(parentFan, elapsedTime * 5.0f, glm::vec3(0.0f, 1.0f, 0.0f));
        childBlade1 = glm::translate(childBlade1, glm::vec3(0.0f, -0.045f, 0.0f));
        shader->setMat4("model", childBlade1);
        shader->setVec3("solidColor", glm::vec3(0.32f, 0.34f, 0.35f));
        fanBladeMesh.draw();

        glm::mat4 childBlade2 = glm::rotate(childBlade1, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        shader->setMat4("model", childBlade2);
        fanBladeMesh.draw();
    }

    glm::mat4 parentBox = glm::translate(glm::mat4(1.0f), breakerPos);
    parentBox = glm::rotate(parentBox, glm::radians(breakerYaw), glm::vec3(0.0f, 1.0f, 0.0f));
    shader->setMat4("model", parentBox);
    shader->setVec3("solidColor", powerRestored ? glm::vec3(0.18f, 0.42f, 0.22f) : glm::vec3(0.42f, 0.16f, 0.16f));
    switchBoxMesh.draw();

    glm::mat4 childLever = glm::translate(parentBox, glm::vec3(0.0f, 0.0f, 0.06f));
    childLever = glm::rotate(childLever, glm::radians(leverAngle), glm::vec3(1.0f, 0.0f, 0.0f));
    childLever = glm::translate(childLever, glm::vec3(0.0f, 0.07f, 0.0f));
    shader->setMat4("model", childLever);
    shader->setVec3("solidColor", glm::vec3(0.85f, 0.78f, 0.20f));
    switchLeverMesh.draw();

    shader->setInt("useSolidColor", 0);

    // Отрисовка билборд-спрайта монстра внутри PS1-буфера
    if (monsterSprite) {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        monsterSprite->Draw(spriteShader, camera.Position, view, projection);
        glDisable(GL_BLEND);
    }

    // 2. Выводим PS1-буфер на основной экран с дизерингом и квантованием цвета
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, width, height);
    glDisable(GL_DEPTH_TEST);

    float distToMonster = glm::distance(glm::vec2(camera.Position.x, camera.Position.z),
                                        glm::vec2(monster.pos.x, monster.pos.z));
    float danger = (monster.state == MonsterState::CHASE)
        ? 1.0f
        : std::clamp((6.0f - distToMonster) / 6.0f, 0.0f, 0.65f);
    danger = std::max(danger, catchFlashTimer);

    ps1PostShader->use();
    ps1PostShader->setInt("screenTexture", 0);
    ps1PostShader->setFloat("dangerIntensity", danger);
    ps1PostShader->setFloat("time", elapsedTime);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, ps1ColorTex);
    glBindVertexArray(screenQuadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);

    renderPDA(width, height);
}