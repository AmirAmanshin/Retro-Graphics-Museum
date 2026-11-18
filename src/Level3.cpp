#include "Levels/Level3.hpp"
#include "Levels/Level4.hpp"
#include "Engine/stb_image.h"
#include <iostream>
#include <random>
#include <algorithm>
#include <cmath>

Level3::Level3(Engine* engine)
    : gameEngine(engine), shader(nullptr), surfaceShader(nullptr),
      spriteShader(nullptr), compassShader(nullptr),
      maze(nullptr), mazeMesh(nullptr), audio(nullptr),
      compassVAO(0), compassVBO(0),
      floorTexture(0), ceilTexture1(0), ceilTexture2(0),
      barrelTexture(0), mannequinTexture(0),
      floorCeilVAO(0), floorCeilVBO(0),
      ceil1VertexCount(0), ceil2VertexCount(0),
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

    Sprite::CleanupSharedMesh();
}

GLuint Level3::loadTexture(const char* path, bool clampToEdge) {
    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    GLint wrapMode = clampToEdge ? GL_CLAMP_TO_EDGE : GL_REPEAT;
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapMode);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapMode);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(true);
    unsigned char* data = stbi_load(path, &width, &height, &nrChannels, 0);
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

    spawnSprites(1.8f, 18);

    audio = new Audio(1);
    audio->playMusic("sounds/lvl3_sound/lvl3_ambient.mp3");
}

void Level3::spawnSprites(float minDistance, int maxCount) {
    sprites.clear();
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
            sprites.emplace_back(glm::vec3(candidate.x, 0.0f, candidate.y), scale, tex);
        }
    }
}

bool Level3::checkCollision(float x, float z) {
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

    return false;
}

void Level3::processInput(GLFWwindow* window, float deltaTime) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
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

    const auto& grid = maze->getGrid();
    int playerGridX = static_cast<int>(camera.Position.x);
    int playerGridZ = static_cast<int>(camera.Position.z);

    if (playerGridZ >= 0 && playerGridZ < static_cast<int>(grid.size()) &&
        playerGridX >= 0 && playerGridX < static_cast<int>(grid[0].size())) {
        if (grid[playerGridZ][playerGridX] == 2) {
            std::cout << "=================================" << std::endl;
            std::cout << "УРОВЕНЬ 3 ПРОЙДЕН! Вы прошли игру!" << std::endl;
            std::cout << "=================================" << std::endl;
            gameEngine->changeLevel(new Level4(gameEngine));
        }
    }
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

    std::vector<size_t> order(sprites.size());
    for (size_t i = 0; i < order.size(); ++i) order[i] = i;

    std::sort(order.begin(), order.end(), [&](size_t a, size_t b) {
        glm::vec3 da = camera.Position - sprites[a].Position;
        glm::vec3 db = camera.Position - sprites[b].Position;
        return glm::dot(da, da) > glm::dot(db, db);
    });

    for (size_t idx : order) {
        sprites[idx].Draw(spriteShader, camera.Position, view, projection);
    }

    glDisable(GL_DEPTH_TEST);

    compassShader->use();
    glm::mat4 orthoProj = glm::ortho(0.0f, (float)width, 0.0f, (float)height);
    compassShader->setMat4("projection", orthoProj);

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