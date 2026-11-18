#include "Levels/Level4.hpp"
#include "Engine/stb_image.h"
#include <iostream>
#include <random>
#include <algorithm>
#include <cmath>
#include <filesystem>

Level4::Level4(Engine* engine)
    : gameEngine(engine), shader(nullptr), compassShader(nullptr),
      maze(nullptr), audio(nullptr),
      compassVAO(0), compassVBO(0),
      floorTexture(0), ceilTexture(0),
      breakerPos(1.5f, 0.52f, 1.5f), breakerYaw(0.0f),
      powerRestored(false), leverAngle(-35.0f),
      flashlightOn(true), fKeyPressed(false), eKeyPressed(false),
      nKeyPressed(false), noclip(false),
      firstMouse(true), lastX(640.0f), lastY(360.0f), elapsedTime(0.0f) {}

Level4::~Level4() {
    if (shader) delete shader;
    if (compassShader) delete compassShader;
    if (maze) delete maze;
    if (audio) delete audio;

    for (int i = 0; i < 4; ++i) wallBatches[i].destroy();
    floorCeilMesh.destroy();
    lampPanelMesh.destroy();
    fanBaseMesh.destroy();
    fanBladeMesh.destroy();
    switchBoxMesh.destroy();
    switchLeverMesh.destroy();

    if (compassVAO) glDeleteVertexArrays(1, &compassVAO);
    if (compassVBO) glDeleteBuffers(1, &compassVBO);

    for (GLuint tex : wallTextures) {
        if (tex != 0) glDeleteTextures(1, &tex);
    }
    if (floorTexture != 0) glDeleteTextures(1, &floorTexture);
    if (ceilTexture != 0) glDeleteTextures(1, &ceilTexture);
}

void Level4::playSoundSafe(const std::string& path) {
    if (audio && std::filesystem::exists(path)) {
        audio->playSound(path.c_str());
    }
}

GLuint Level4::loadTexture(const char* path) {
    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(true);
    unsigned char* data = stbi_load(path, &width, &height, &nrChannels, 0);
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

void Level4::init() {
    camera = Camera(glm::vec3(1.5f, 0.5f, 1.5f));
    camera.AllowPitch = true;
    camera.Pitch = 0.0f;

    shader = new Shader("shaders/level4.vert", "shaders/level4.frag");
    compassShader = new Shader("shaders/compass.vert", "shaders/compass.frag");

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

    float compassVertices[] = { 0.0f, 15.0f, -10.0f, -10.0f, 10.0f, -10.0f };
    glGenVertexArrays(1, &compassVAO);
    glGenBuffers(1, &compassVBO);
    glBindVertexArray(compassVAO);
    glBindBuffer(GL_ARRAY_BUFFER, compassVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(compassVertices), compassVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);

    setupLampsAndTriggers();
    audio = new Audio(1);
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

            if (grid[z - 1][x] == 1) {
                wallMounts.push_back({ glm::vec3(cx, 0.52f, static_cast<float>(z) + 0.05f), 0.0f, dist });
            }
            if (grid[z + 1][x] == 1) {
                wallMounts.push_back({ glm::vec3(cx, 0.52f, static_cast<float>(z + 1) - 0.05f), 180.0f, dist });
            }
            if (grid[z][x - 1] == 1) {
                wallMounts.push_back({ glm::vec3(static_cast<float>(x) + 0.05f, 0.52f, cz), 90.0f, dist });
            }
            if (grid[z][x + 1] == 1) {
                wallMounts.push_back({ glm::vec3(static_cast<float>(x + 1) - 0.05f, 0.52f, cz), -90.0f, dist });
            }
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
    if (!powerRestored) {
        return "TASK: USE BREAKER [E] FOR EXIT";
    }
    return "EXIT POWERED! FOLLOW COMPASS";
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
        glfwSetWindowShouldClose(window, true);
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

    float velocity = camera.MovementSpeed * deltaTime * (noclip ? 2.0f : 1.0f);
    glm::vec3 moveDir(0.0f);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) moveDir += camera.Front;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) moveDir -= camera.Front;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) moveDir -= camera.Right;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) moveDir += camera.Right;

    static float stepTimer = 0.45f;
    if (glm::length(moveDir) > 0.0f) {
        if (noclip) {
            camera.Position += glm::normalize(moveDir) * velocity;
        } else {
            if (camera.IsGrounded) {
                stepTimer += deltaTime;
                if (stepTimer > 0.45f) {
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
        stepTimer = 0.45f;
    }
}

void Level4::update(float deltaTime) {
    elapsedTime += deltaTime;
    if (!noclip) {
        camera.UpdatePhysics(deltaTime);
    }

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

    for (auto& trig : soundTriggers) {
        if (!trig.triggered && glm::distance(camera.Position, trig.position) <= trig.radius) {
            trig.triggered = true;
            playSoundSafe(trig.soundPath);
        }
    }

    const auto& grid = maze->getGrid();
    int gx = static_cast<int>(camera.Position.x);
    int gz = static_cast<int>(camera.Position.z);
    if (powerRestored && gz >= 0 && gz < static_cast<int>(grid.size()) &&
        gx >= 0 && gx < static_cast<int>(grid[0].size()) && grid[gz][gx] == 2) {
        std::cout << "LEVEL 4 COMPLETE! Demo finished." << std::endl;
        glfwSetWindowShouldClose(glfwGetCurrentContext(), true);
    }
}

void Level4::render() {
    int width, height;
    glfwGetWindowSize(glfwGetCurrentContext(), &width, &height);

    glClearColor(0.01f, 0.01f, 0.015f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);

    glm::mat4 projection = glm::perspective(glm::radians(60.0f), (float)width / (float)height, 0.1f, 100.0f);
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

    glDisable(GL_DEPTH_TEST);
    compassShader->use();
    glm::mat4 orthoProj = glm::ortho(0.0f, (float)width, 0.0f, (float)height);
    compassShader->setMat4("projection", orthoProj);

    float exitX = maze->getExitX() + 0.5f;
    float exitZ = maze->getExitY() + 0.5f;
    float rotation = glm::radians(camera.Yaw) - std::atan2(exitZ - camera.Position.z, exitX - camera.Position.x);

    glm::mat4 modelCompass = glm::translate(glm::mat4(1.0f), glm::vec3(width - 70.0f, 70.0f, 0.0f));
    modelCompass = glm::scale(modelCompass, glm::vec3(1.5f, 1.5f, 1.0f));
    modelCompass = glm::rotate(modelCompass, rotation, glm::vec3(0.0f, 0.0f, 1.0f));
    compassShader->setMat4("model", modelCompass);

    glBindVertexArray(compassVAO);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glBindVertexArray(0);
    glEnable(GL_DEPTH_TEST);
}