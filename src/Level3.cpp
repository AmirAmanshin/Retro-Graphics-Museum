#include "Levels/Level3.hpp"
#include "Levels/Level4.hpp"
#include "Levels/MainMenuLevel.hpp"
#include "Engine/stb_image.h"
#include <iostream>
#include <random>
#include <algorithm>
#include <cmath>
#include <filesystem>

Level3::Level3(Engine* engine)
    : gameEngine(engine), shader(nullptr), surfaceShader(nullptr),
      spriteShader(nullptr), compassShader(nullptr),
      maze(nullptr), mazeMesh(nullptr), audio(nullptr),
      compassVAO(0), compassVBO(0), crosshairVAO(0), crosshairVBO(0),
      floorTexture(0), ceilTexture1(0), ceilTexture2(0),
      barrelTexture(0), mannequinTexture(0),
      shotgunIdleTex(0), shotgunFireTex(0), shotgunReloadTex(0),
      floorCeilVAO(0), floorCeilVBO(0),
      ceil1VertexCount(0), ceil2VertexCount(0),
      shotgunState(ShotgunState::IDLE), shellsInClip(2),
      shotgunTimer(0.0f), weaponBobTimer(0.0f),
      mouseLeftPressed(false), rKeyPressed(false),
      firstMouse(true), lastX(635.0f), lastY(360.0f),
      noclip(false), fKeyPressed(false) {}

Level3::~Level3() {
    if (shader) delete shader;
    if (surfaceShader) delete surfaceShader;
    if (spriteShader) delete spriteShader;
    if (compassShader) delete compassShader;
    if (maze) delete maze;
    if (mazeMesh) delete mazeMesh;
    if (audio) delete audio;

    if (compassVAO) glDeleteVertexArrays(1, &compassVAO);
    if (compassVBO) glDeleteBuffers(1, &compassVBO);
    if (crosshairVAO) glDeleteVertexArrays(1, &crosshairVAO);
    if (crosshairVBO) glDeleteBuffers(1, &crosshairVBO);
    if (floorCeilVAO) glDeleteVertexArrays(1, &floorCeilVAO);
    if (floorCeilVBO) glDeleteBuffers(1, &floorCeilVBO);

    for (GLuint tex : textures) {
        if (tex != 0) glDeleteTextures(1, &tex);
    }
    if (floorTexture != 0) glDeleteTextures(1, &floorTexture);
    if (ceilTexture1 != 0) glDeleteTextures(1, &ceilTexture1);
    if (ceilTexture2 != 0) glDeleteTextures(1, &ceilTexture2);
    if (barrelTexture != 0) glDeleteTextures(1, &barrelTexture);
    if (mannequinTexture != 0) glDeleteTextures(1, &mannequinTexture);
    if (shotgunIdleTex != 0) glDeleteTextures(1, &shotgunIdleTex);
    if (shotgunFireTex != 0) glDeleteTextures(1, &shotgunFireTex);
    if (shotgunReloadTex != 0) glDeleteTextures(1, &shotgunReloadTex);

    Sprite::CleanupSharedMesh();
}

void Level3::playSoundSafe(const std::string& path) {
    if (audio && std::filesystem::exists(path)) {
        audio->playSound(path.c_str());
    }
}

// frameType: 0 = Idle, 1 = Fire (с вспышкой), 2 = Reload (перелом стволов + 2 красные гильзы)
GLuint Level3::createProceduralShotgunTexture(int frameType) {
    const int w = 64;
    const int h = 64;
    std::vector<unsigned char> px(w * h * 4, 0);

    auto setPixel = [&](int x, int y, unsigned char r, unsigned char g, unsigned char b, unsigned char a = 255) {
        if (x < 0 || x >= w || y < 0 || y >= h) return;
        int idx = (y * w + x) * 4;
        px[idx + 0] = r;
        px[idx + 1] = g;
        px[idx + 2] = b;
        px[idx + 3] = a;
    };

    if (frameType == 0 || frameType == 1) {
        int yShift = (frameType == 1) ? -4 : 0;

        // Вспышка выстрела (рисуем за стволами)
        if (frameType == 1) {
            int cx = 32;
            int cy = 44;
            for (int y = 28; y < 63; ++y) {
                for (int x = 10; x < 54; ++x) {
                    float dx = static_cast<float>(x - cx);
                    float dy = static_cast<float>(y - cy);
                    float dist = std::sqrt(dx * dx + dy * dy);
                    float angle = std::atan2(dy, dx);
                    float rays = 1.0f + 0.35f * std::cos(angle * 7.0f) + 0.2f * std::sin(angle * 11.0f);
                    float rNorm = dist / (18.0f * rays);

                    if (rNorm < 0.42f) {
                        setPixel(x, y, 255, 255, 220, 255); // Ядро вспышки
                    } else if (rNorm < 0.72f) {
                        setPixel(x, y, 255, 195, 45, 245);  // Жёлтый огонь
                    } else if (rNorm < 0.98f) {
                        setPixel(x, y, 235, 85, 20, 210);   // Оранжевые края
                    }
                }
            }
        }

        // Деревянный приклад / цевьё (внизу)
        for (int y = 0; y <= 17 + yShift; ++y) {
            int halfW = 10 - (y / 6);
            for (int x = 32 - halfW; x <= 32 + halfW; ++x) {
                unsigned char shade = static_cast<unsigned char>(95 + ((x + y * 2) % 3) * 14);
                setPixel(x, y, shade, static_cast<unsigned char>(shade * 0.56f), static_cast<unsigned char>(shade * 0.30f));
            }
        }

        // Два спаренных ствола двустволки
        for (int y = 14 + yShift; y <= 43 + yShift; ++y) {
            int taper = (y - (14 + yShift)) / 10; // лёгкое сужение к дулу
            int leftStart  = 24 + taper;
            int leftEnd    = 31;
            int rightStart = 32;
            int rightEnd   = 39 - taper;

            // Левый ствол
            for (int x = leftStart; x <= leftEnd; ++x) {
                bool highlight = (x == leftStart + 2 || x == leftStart + 3);
                unsigned char c = highlight ? 128 : 68;
                setPixel(x, y, c, c, static_cast<unsigned char>(c + 8));
            }
            // Правый ствол
            for (int x = rightStart; x <= rightEnd; ++x) {
                bool highlight = (x == rightEnd - 2 || x == rightEnd - 3);
                unsigned char c = highlight ? 128 : 68;
                setPixel(x, y, c, c, static_cast<unsigned char>(c + 8));
            }
            // Центральная прицельная планка между стволами
            setPixel(31, y, 42, 42, 46);
            setPixel(32, y, 42, 42, 46);
        }

        // Мушка на конце стволов
        setPixel(31, 44 + yShift, 190, 175, 90);
        setPixel(32, 44 + yShift, 190, 175, 90);
    } else {
        // frameType == 2: Перезарядка (переломленные стволы + 2 красных патрона)
        // Приклад снизу
        for (int y = 0; y <= 18; ++y) {
            for (int x = 22; x <= 41; ++x) {
                unsigned char shade = static_cast<unsigned char>(92 + ((x * 3 + y) % 3) * 12);
                setPixel(x, y, shade, static_cast<unsigned char>(shade * 0.55f), static_cast<unsigned char>(shade * 0.28f));
            }
        }

        // Казённик (открытый узел перелома)
        for (int y = 18; y <= 25; ++y) {
            for (int x = 23; x <= 40; ++x) {
                setPixel(x, y, 52, 54, 60);
            }
        }

        // Два красных патрона 12-го калибра с латунными донцами
        for (int shell = 0; shell < 2; ++shell) {
            int sx = (shell == 0) ? 26 : 34;
            for (int y = 24; y <= 33; ++y) {
                for (int x = sx; x <= sx + 3; ++x) {
                    if (y <= 26) {
                        setPixel(x, y, 225, 185, 55); // Латунь
                    } else {
                        setPixel(x, y, 205, 32, 32);  // Красная гильза
                    }
                }
            }
        }

        // Откинутые вперёд и вниз стволы
        for (int y = 10; y <= 23; ++y) {
            for (int x = 24; x <= 39; ++x) {
                if (x == 31 || x == 32) {
                    setPixel(x, y, 35, 35, 40);
                } else {
                    setPixel(x, y, 82, 85, 92);
                }
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
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, px.data());
    return texID;
}

GLuint Level3::loadTexture(const char* path, bool clampToEdge, bool allowMissing) {
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
        GLenum format = GL_RGB;
        if (nrChannels == 1) format = GL_RED;
        else if (nrChannels == 4) format = GL_RGBA;

        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        stbi_image_free(data);
    } else {
        std::cout << "Failed to load texture at path: " << path << std::endl;
        unsigned char fallback[4] = { 90, 90, 95, 255 };
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, fallback);
    }

    return textureID;
}

void Level3::init() {
    camera = Camera(glm::vec3(1.5f, 0.5f, 1.5f));
    camera.AllowPitch = true;
    camera.Pitch = 0.0f;

    shader = new Shader("shaders/level2.vert", "shaders/level2.frag");
    surfaceShader = new Shader("shaders/level3.vert", "shaders/level3.frag");
    spriteShader = new Shader("shaders/sprite.vert", "shaders/sprite.frag");
    compassShader = new Shader("shaders/compass.vert", "shaders/compass.frag");

    const int mazeSize = 25;
    maze = new MazeGenerator(mazeSize, mazeSize);
    maze->generate();

    mazeMesh = new MazeMesh();
    mazeMesh->buildTextured(maze->getGrid());

    textures.push_back(loadTexture("textures/lvl3_tex/lvl3_wall_2.png"));
    textures.push_back(loadTexture("textures/lvl3_tex/lvl3_wall_2.png"));
    textures.push_back(loadTexture("textures/lvl3_tex/lvl3_wall_1.png"));
    textures.push_back(loadTexture("textures/lvl3_tex/lvl3_exit.png"));

    floorTexture = loadTexture("textures/lvl3_tex/lvl3_floor.png");
    ceilTexture1 = loadTexture("textures/lvl3_tex/lvl3_ceiling_1.png");
    ceilTexture2 = loadTexture("textures/lvl3_tex/lvl3_ceiling_2.png");

    barrelTexture = loadTexture("textures/lvl3_tex/lvl3_sprite_1.png", true);
    mannequinTexture = loadTexture("textures/lvl3_tex/lvl3_sprite_2.png", true);

    // Загрузка текстур двустволки (или процедурный Doom-пиксельарт, если файлов пока нет)
    shotgunIdleTex = loadTexture("textures/lvl3_tex/shotgun_idle.png", true, true);
    if (shotgunIdleTex == 0) shotgunIdleTex = createProceduralShotgunTexture(0);

    shotgunFireTex = loadTexture("textures/lvl3_tex/shotgun_fire.png", true, true);
    if (shotgunFireTex == 0) shotgunFireTex = createProceduralShotgunTexture(1);

    shotgunReloadTex = loadTexture("textures/lvl3_tex/shotgun_reload.png", true, true);
    if (shotgunReloadTex == 0) shotgunReloadTex = createProceduralShotgunTexture(2);

    shader->use();
    shader->setInt("texWall1", 0);
    shader->setInt("texWall2", 1);
    shader->setInt("texWall3", 2);
    shader->setInt("texExit", 3);

    surfaceShader->use();
    surfaceShader->setInt("texWall1", 0);

    float s = static_cast<float>(mazeSize);
    std::vector<float> vertices = {
        0.0f, 0.0f, 0.0f,  0.0f, 0.0f,
           s, 0.0f, 0.0f,     s, 0.0f,
           s, 0.0f,    s,     s,    s,
        0.0f, 0.0f, 0.0f,  0.0f, 0.0f,
           s, 0.0f,    s,     s,    s,
        0.0f, 0.0f,    s,  0.0f,    s
    };

    auto appendCellCeiling = [&](int x, int z) {
        float x0 = static_cast<float>(x);
        float z0 = static_cast<float>(z);
        float x1 = x0 + 1.0f;
        float z1 = z0 + 1.0f;
        float cell[] = {
            x0, 1.0f, z0,  0.0f, 0.0f,
            x1, 1.0f, z1,  1.0f, 1.0f,
            x1, 1.0f, z0,  1.0f, 0.0f,
            x0, 1.0f, z0,  0.0f, 0.0f,
            x0, 1.0f, z1,  0.0f, 1.0f,
            x1, 1.0f, z1,  1.0f, 1.0f
        };
        vertices.insert(vertices.end(), std::begin(cell), std::end(cell));
    };

    std::mt19937 ceilGen(std::random_device{}());
    std::uniform_int_distribution<int> ceilDist(0, 99);

    std::vector<glm::ivec2> ceil1Cells;
    std::vector<glm::ivec2> ceil2Cells;

    for (int z = 0; z < mazeSize; ++z) {
        for (int x = 0; x < mazeSize; ++x) {
            if (ceilDist(ceilGen) < 10) {
                ceil2Cells.emplace_back(x, z);
            } else {
                ceil1Cells.emplace_back(x, z);
            }
        }
    }

    ceil1VertexCount = 0;
    for (const auto& cell : ceil1Cells) {
        appendCellCeiling(cell.x, cell.y);
        ceil1VertexCount += 6;
    }

    ceil2VertexCount = 0;
    for (const auto& cell : ceil2Cells) {
        appendCellCeiling(cell.x, cell.y);
        ceil2VertexCount += 6;
    }

    glGenVertexArrays(1, &floorCeilVAO);
    glGenBuffers(1, &floorCeilVBO);

    glBindVertexArray(floorCeilVAO);
    glBindBuffer(GL_ARRAY_BUFFER, floorCeilVBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);

    float compassVertices[] = {
         0.0f,  15.0f,
        -10.0f, -10.0f,
         10.0f, -10.0f
    };

    glGenVertexArrays(1, &compassVAO);
    glGenBuffers(1, &compassVBO);

    glBindVertexArray(compassVAO);
    glBindBuffer(GL_ARRAY_BUFFER, compassVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(compassVertices), compassVertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);

    // Крестик прицела (2 пересекающихся прямоугольника)
    float crossVerts[] = {
        -6.0f, -1.0f,   6.0f, -1.0f,   6.0f,  1.0f,
        -6.0f, -1.0f,   6.0f,  1.0f,  -6.0f,  1.0f,
        -1.0f, -6.0f,   1.0f, -6.0f,   1.0f,  6.0f,
        -1.0f, -6.0f,   1.0f,  6.0f,  -1.0f,  6.0f
    };

    glGenVertexArrays(1, &crosshairVAO);
    glGenBuffers(1, &crosshairVBO);
    glBindVertexArray(crosshairVAO);
    glBindBuffer(GL_ARRAY_BUFFER, crosshairVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(crossVerts), crossVerts, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);

    spawnSprites(1.8f, 22);

    audio = new Audio(1);
    audio->playMusic("sounds/lvl3_sound/lvl3_ambient.mp3");
}

void Level3::spawnSprites(float minDistance, int maxCount) {
    entities.clear();
    const auto& grid = maze->getGrid();
    glm::vec2 startPos(1.5f, 1.5f);

    std::vector<glm::vec2> candidates;
    for (int z = 1; z < static_cast<int>(grid.size()) - 1; ++z) {
        for (int x = 1; x < static_cast<int>(grid[z].size()) - 1; ++x) {
            if (grid[z][x] == 0) {
                glm::vec2 pos(static_cast<float>(x) + 0.5f, static_cast<float>(z) + 0.5f);
                if (glm::distance(pos, startPos) >= minDistance) {
                    candidates.push_back(pos);
                }
            }
        }
    }

    std::random_device rd;
    std::mt19937 gen(rd());
    std::shuffle(candidates.begin(), candidates.end(), gen);
    std::uniform_int_distribution<int> typeDist(0, 1);

    std::vector<glm::vec2> placed;
    for (const auto& candidate : candidates) {
        if (static_cast<int>(placed.size()) >= maxCount) break;

        bool tooClose = false;
        for (const auto& existing : placed) {
            if (glm::distance(candidate, existing) < minDistance) {
                tooClose = true;
                break;
            }
        }

        if (!tooClose) {
            placed.push_back(candidate);
            bool isMannequin = (typeDist(gen) == 1);
            GLuint tex = isMannequin ? mannequinTexture : barrelTexture;
            glm::vec2 scale = isMannequin ? glm::vec2(0.65f, 0.90f) : glm::vec2(0.55f, 0.58f);

            entities.push_back({
                Sprite(glm::vec3(candidate.x, 0.0f, candidate.y), scale, tex),
                isMannequin,
                isMannequin ? 2 : 1, // Манекен выдерживает 2 выстрела из двустволки
                0.0f,
                false
            });
        }
    }
}

bool Level3::checkCollision(float x, float z) const {
    float radius = 0.2f;
    const auto& grid = maze->getGrid();

    int minX = static_cast<int>(x - radius);
    int maxX = static_cast<int>(x + radius);
    int minZ = static_cast<int>(z - radius);
    int maxZ = static_cast<int>(z + radius);

    if (minX < 0 || maxX >= static_cast<int>(grid[0].size()) ||
        minZ < 0 || maxZ >= static_cast<int>(grid.size())) return true;

    if (grid[minZ][minX] == 1) return true;
    if (grid[minZ][maxX] == 1) return true;
    if (grid[maxZ][minX] == 1) return true;
    if (grid[maxZ][maxX] == 1) return true;

    // Неразрушенные манекены (второй спрайт) блокируют проход
    glm::vec2 playerPos(x, z);
    const float blockRadius = 0.48f; // радиус игрока (0.20) + радиус манекена (0.28)
    for (const auto& ent : entities) {
        if (ent.isMannequin && !ent.destroyed) {
            glm::vec2 entPos(ent.sprite.Position.x, ent.sprite.Position.z);
            if (glm::distance(playerPos, entPos) < blockRadius) {
                return true;
            }
        }
    }

    return false;
}

bool Level3::hasLineOfSight(const glm::vec3& from, const glm::vec3& to) const {
    const auto& grid = maze->getGrid();
    int rows = static_cast<int>(grid.size());
    int cols = static_cast<int>(grid[0].size());

    glm::vec2 a(from.x, from.z);
    glm::vec2 b(to.x, to.z);
    float dist = glm::distance(a, b);
    int steps = static_cast<int>(dist * 6.0f);
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

void Level3::tryShootShotgun() {
    // Нельзя стрелять во время кулдауна выстрела или перезарядки
    if (shotgunState != ShotgunState::IDLE || shellsInClip <= 0) {
        return;
    }

    shellsInClip--;
    shotgunState = ShotgunState::FIRING;
    shotgunTimer = 0.24f; // Кулдаун между выстрелами стволов

    playSoundSafe("sounds/lvl3_sound/shotgun_fire.wav");
    if (!std::filesystem::exists("sounds/lvl3_sound/shotgun_fire.wav")) {
        playSoundSafe("sounds/lvl3_sound/lvl3_footstep.wav");
    }

    // Проверка попадания в ближайший манекен по линии взгляда
    glm::vec2 fwd2D(camera.Front.x, camera.Front.z);
    if (glm::length(fwd2D) > 0.0001f) {
        fwd2D = glm::normalize(fwd2D);
    }

    int bestIdx = -1;
    float bestDist = 4.2f; // Максимальная дальность дроби

    for (size_t i = 0; i < entities.size(); ++i) {
        auto& ent = entities[i];
        if (!ent.isMannequin || ent.destroyed) continue;

        glm::vec2 toEnt(ent.sprite.Position.x - camera.Position.x,
                        ent.sprite.Position.z - camera.Position.z);
        float forwardDist = glm::dot(toEnt, fwd2D);
        if (forwardDist <= 0.05f || forwardDist >= bestDist) continue;

        float perpDist = std::abs(toEnt.x * fwd2D.y - toEnt.y * fwd2D.x);
        if (perpDist <= 0.42f && hasLineOfSight(camera.Position, ent.sprite.Position)) {
            bestDist = forwardDist;
            bestIdx = static_cast<int>(i);
        }
    }

    if (bestIdx != -1) {
        auto& target = entities[bestIdx];
        target.hp -= 1;
        target.hitFlashTimer = 0.16f;

        if (target.hp <= 0) {
            target.destroyed = true;
            playSoundSafe("sounds/lvl3_sound/mannequin_break.wav");
            std::cout << "Mannequin destroyed! Path clear." << std::endl;
        } else {
            std::cout << "Mannequin hit! (1 more shot needed)" << std::endl;
        }
    }
}

void Level3::processInput(GLFWwindow* window, float deltaTime) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        gameEngine->changeLevel(new MainMenuLevel(gameEngine));
        return;
    }

    if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS) {
        if (!fKeyPressed) {
            noclip = !noclip;
            fKeyPressed = true;
            std::cout << "Level 3 Noclip: " << (noclip ? "ON" : "OFF") << std::endl;
        }
    } else {
        fKeyPressed = false;
    }

    // Выстрел из двустволки по ЛКМ (или Enter)
    bool shootPressed = (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) ||
                        (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS);
    if (shootPressed) {
        if (!mouseLeftPressed) {
            tryShootShotgun();
            mouseLeftPressed = true;
        }
    } else {
        mouseLeftPressed = false;
    }

    // Ручная перезарядка на R, если один патрон уже отстрелян
    if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
        if (!rKeyPressed) {
            if (shotgunState == ShotgunState::IDLE && shellsInClip < 2) {
                shotgunState = ShotgunState::RELOADING;
                shotgunTimer = 0.85f;
                playSoundSafe("sounds/lvl3_sound/shotgun_reload.wav");
            }
            rKeyPressed = true;
        }
    } else {
        rKeyPressed = false;
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

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);

    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)  camera.ProcessMouseMovement(-150.0f * deltaTime, 0.0f);
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) camera.ProcessMouseMovement(150.0f * deltaTime, 0.0f);
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)    camera.ProcessMouseMovement(0.0f, 120.0f * deltaTime);
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)  camera.ProcessMouseMovement(0.0f, -120.0f * deltaTime);

    float velocity = camera.MovementSpeed * deltaTime * (noclip ? 2.0f : 1.0f);
    glm::vec3 moveDir(0.0f);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) moveDir += camera.Front;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) moveDir -= camera.Front;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) moveDir -= camera.Right;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) moveDir += camera.Right;

    static float stepTimer = 0.4f;

    if (glm::length(moveDir) > 0.0f) {
        weaponBobTimer += deltaTime * 9.0f;

        if (noclip) {
            camera.Position += glm::normalize(moveDir) * velocity;
        } else {
            if (camera.IsGrounded) {
                stepTimer += deltaTime;
                if (stepTimer > 0.4f) {
                    audio->playSound("sounds/lvl3_sound/lvl3_footstep.wav");
                    stepTimer = 0.0f;
                }
            }

            moveDir.y = 0.0f;
            if (glm::length(moveDir) > 0.0001f) {
                moveDir = glm::normalize(moveDir) * velocity;

                if (!checkCollision(camera.Position.x + moveDir.x, camera.Position.z)) {
                    camera.Position.x += moveDir.x;
                }
                if (!checkCollision(camera.Position.x, camera.Position.z + moveDir.z)) {
                    camera.Position.z += moveDir.z;
                }
            }
        }
    } else {
        stepTimer = 0.4f;
    }
}

void Level3::update(float deltaTime) {
    if (!noclip) {
        camera.UpdatePhysics(deltaTime);
    }

    // Таймеры состояния и перезарядки двустволки
    if (shotgunTimer > 0.0f) {
        shotgunTimer -= deltaTime;
        if (shotgunTimer <= 0.0f) {
            shotgunTimer = 0.0f;
            if (shotgunState == ShotgunState::FIRING) {
                if (shellsInClip <= 0) {
                    // Оба ствола пусты — запускаем перезарядку с кулдауном
                    shotgunState = ShotgunState::RELOADING;
                    shotgunTimer = 0.85f;
                    playSoundSafe("sounds/lvl3_sound/shotgun_reload.wav");
                } else {
                    shotgunState = ShotgunState::IDLE;
                }
            } else if (shotgunState == ShotgunState::RELOADING) {
                shellsInClip = 2;
                shotgunState = ShotgunState::IDLE;
            }
        }
    }

    // Анимация вздрагивания подстреленного манекена
    for (auto& ent : entities) {
        if (ent.hitFlashTimer > 0.0f) {
            ent.hitFlashTimer = std::max(0.0f, ent.hitFlashTimer - deltaTime);
        }
    }

    const auto& grid = maze->getGrid();
    int playerGridX = static_cast<int>(camera.Position.x);
    int playerGridZ = static_cast<int>(camera.Position.z);

    if (playerGridZ >= 0 && playerGridZ < static_cast<int>(grid.size()) &&
        playerGridX >= 0 && playerGridX < static_cast<int>(grid[0].size())) {
        if (grid[playerGridZ][playerGridX] == 2) {
            std::cout << "=================================" << std::endl;
            std::cout << "УРОВЕНЬ 3 ПРОЙДЕН! Переход на Эру 4!" << std::endl;
            std::cout << "=================================" << std::endl;
            gameEngine->changeLevel(new Level4(gameEngine));
        }
    }
}

void Level3::renderShotgunHUD(int width, int height) {
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glm::mat4 orthoProj = glm::ortho(0.0f, static_cast<float>(width), 0.0f, static_cast<float>(height));

    // 1. Прицел в центре экрана
    compassShader->use();
    compassShader->setMat4("projection", orthoProj);
    glm::mat4 crossModel = glm::translate(glm::mat4(1.0f), glm::vec3(width * 0.5f, height * 0.5f, 0.0f));
    compassShader->setMat4("model", crossModel);
    compassShader->setVec4("uColor", glm::vec4(0.85f, 1.0f, 0.85f, 0.80f));

    glBindVertexArray(crosshairVAO);
    glDrawArrays(GL_TRIANGLES, 0, 12);
    glBindVertexArray(0);

    // 2. Спрайт двустволки снизу по центру (в стиле Doom)
    GLuint activeGunTex = shotgunIdleTex;
    float offsetY = 0.0f;
    float offsetX = std::sin(weaponBobTimer) * 14.0f;
    offsetY -= std::abs(std::cos(weaponBobTimer)) * 8.0f;

    if (shotgunState == ShotgunState::FIRING) {
        activeGunTex = shotgunFireTex;
        offsetY -= 18.0f * (shotgunTimer / 0.24f); // Отдача вниз при выстреле
    } else if (shotgunState == ShotgunState::RELOADING) {
        activeGunTex = shotgunReloadTex;
        float reloadWave = std::sin((1.0f - shotgunTimer / 0.85f) * glm::pi<float>());
        offsetY -= 26.0f * reloadWave;
    }

    float gunSize = static_cast<float>(height) * 0.52f;
    Sprite gunView(
        glm::vec3(width * 0.5f + offsetX, offsetY, 0.0f),
        glm::vec2(gunSize, gunSize),
        activeGunTex
    );
    // Передаём единичные cameraPos/view — угол поворота станет 0, и Sprite нарисуется как 2D HUD
    gunView.Draw(spriteShader, glm::vec3(width * 0.5f + offsetX, offsetY, 1.0f), glm::mat4(1.0f), orthoProj);

    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
}

void Level3::render() {
    int width, height;
    glfwGetWindowSize(glfwGetCurrentContext(), &width, &height);

    glClearColor(0.05f, 0.05f, 0.07f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glEnable(GL_DEPTH_TEST);

    glm::mat4 projection = glm::perspective(glm::radians(60.0f), (float)width / (float)height, 0.1f, 100.0f);
    glm::mat4 view = camera.GetViewMatrix();
    glm::mat4 model = glm::mat4(1.0f);

    surfaceShader->use();
    surfaceShader->setMat4("projection", projection);
    surfaceShader->setMat4("view", view);
    surfaceShader->setMat4("model", model);
    surfaceShader->setInt("texIndex", 0);

    glBindVertexArray(floorCeilVAO);
    glActiveTexture(GL_TEXTURE0);

    glBindTexture(GL_TEXTURE_2D, floorTexture);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    glBindTexture(GL_TEXTURE_2D, ceilTexture1);
    glDrawArrays(GL_TRIANGLES, 6, ceil1VertexCount);

    glBindTexture(GL_TEXTURE_2D, ceilTexture2);
    glDrawArrays(GL_TRIANGLES, 6 + ceil1VertexCount, ceil2VertexCount);
    glBindVertexArray(0);

    shader->use();
    shader->setMat4("projection", projection);
    shader->setMat4("view", view);
    shader->setMat4("model", model);
    mazeMesh->drawTextured(textures);

    std::vector<size_t> order;
    order.reserve(entities.size());
    for (size_t i = 0; i < entities.size(); ++i) {
        if (!entities[i].destroyed) {
            order.push_back(i);
        }
    }

    std::sort(order.begin(), order.end(), [&](size_t a, size_t b) {
        glm::vec3 da = camera.Position - entities[a].sprite.Position;
        glm::vec3 db = camera.Position - entities[b].sprite.Position;
        return glm::dot(da, da) > glm::dot(db, db);
    });

    for (size_t idx : order) {
        auto& ent = entities[idx];
        if (ent.hitFlashTimer > 0.0f) {
            // Кратковременное сжатие/отшатывание манекена после первого выстрела
            Sprite hitCopy = ent.sprite;
            hitCopy.Scale.y *= 0.86f;
            hitCopy.Position.x += std::sin(ent.hitFlashTimer * 60.0f) * 0.05f;
            hitCopy.Draw(spriteShader, camera.Position, view, projection);
        } else {
            ent.sprite.Draw(spriteShader, camera.Position, view, projection);
        }
    }

    // Отрисовка двустволки и прицела поверх 3D-мира
    renderShotgunHUD(width, height);

    // Отрисовка компаса в правом нижнем углу
    glDisable(GL_DEPTH_TEST);

    compassShader->use();
    glm::mat4 orthoProj = glm::ortho(0.0f, (float)width, 0.0f, (float)height);
    compassShader->setMat4("projection", orthoProj);
    compassShader->setVec4("uColor", glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));

    float exitX = maze->getExitX() * 1.0f + 0.5f;
    float exitZ = maze->getExitY() * 1.0f + 0.5f;

    float dirX = exitX - camera.Position.x;
    float dirZ = exitZ - camera.Position.z;
    float angleToExit = std::atan2(dirZ, dirX);
    float cameraAngle = glm::radians(camera.Yaw);

    float rotation = cameraAngle - angleToExit;

    glm::mat4 modelCompass = glm::mat4(1.0f);
    modelCompass = glm::translate(modelCompass, glm::vec3(width - 70.0f, 70.0f, 0.0f));
    modelCompass = glm::scale(modelCompass, glm::vec3(1.5f, 1.5f, 1.0f));
    modelCompass = glm::rotate(modelCompass, rotation, glm::vec3(0.0f, 0.0f, 1.0f));

    compassShader->setMat4("model", modelCompass);

    glBindVertexArray(compassVAO);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glBindVertexArray(0);

    glEnable(GL_DEPTH_TEST);
}