#include <Level1.hpp>
#include <iostream>
#include <cmath>

Level1::Level1(Engine* engine) 
    : gameEngine(engine), shader(nullptr), maze(nullptr), mazeMesh(nullptr), 
      noclip(false), fKeyPressed(false) {}

Level1::~Level1() {
    if (shader) delete shader;
    if (maze) delete maze;
    if (mazeMesh) delete mazeMesh;
}

void Level1::init() {
    camera = Camera(glm::vec3(1.5f, 0.5f, 1.5f));
    targetPos = camera.Position;
    targetYaw = -90.0f;
    camera.Yaw = targetYaw;

    shader = new Shader("shaders/level1.vert", "shaders/level1.frag");

    maze = new MazeGenerator(21, 21);
    maze->generate();
    maze->printToConsole();

    mazeMesh = new MazeMesh();
    mazeMesh->build(maze->getGrid());
}

bool Level1::checkCollision(float x, float z) {
    float radius = 0.2f;
    const auto& grid = maze->getGrid();
    
    int minX = static_cast<int>(x - radius);
    int maxX = static_cast<int>(x + radius);
    int minZ = static_cast<int>(z - radius);
    int maxZ = static_cast<int>(z + radius);
    
    if (minX < 0 || maxX >= grid[0].size() || minZ < 0 || maxZ >= grid.size()) return true;
    
    if (grid[minZ][minX] == 1) return true;
    if (grid[minZ][maxX] == 1) return true;
    if (grid[maxZ][minX] == 1) return true;
    if (grid[maxZ][maxX] == 1) return true;
    
    return false;
}

void Level1::processInput(GLFWwindow* window, float deltaTime) {
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }

    if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS) {
        if (!fKeyPressed) {
            noclip = !noclip;
            fKeyPressed = true;
            
            if (!noclip) {
                targetPos.x = std::floor(camera.Position.x) + 0.5f;
                targetPos.z = std::floor(camera.Position.z) + 0.5f;
                targetPos.y = 0.5f;
                
                targetYaw = std::round(camera.Yaw / 90.0f) * 90.0f;
                
                camera.SetPose(targetPos, targetYaw);
            }
            std::cout << "Noclip: " << (noclip ? "ON" : "OFF") << std::endl;
        }
    } else {
        fKeyPressed = false;
    }

    // Noclip
    if (noclip) {
        float velocity = camera.MovementSpeed * deltaTime * 2.0f;
        glm::vec3 moveDir(0.0f);
        
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) moveDir += camera.Front;
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) moveDir -= camera.Front;
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) moveDir -= camera.Right;
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) moveDir += camera.Right;
        
        if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) camera.SetPose(camera.Position, camera.Yaw - 100.0f * deltaTime);
        if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) camera.SetPose(camera.Position, camera.Yaw + 100.0f * deltaTime);

        if (glm::length(moveDir) > 0.0f) {
            camera.Position += glm::normalize(moveDir) * velocity;
        }
        return;
    }

    // Rules for Level 1
    
    // Longing neck
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) targetPos.y = 2.0f;
    else targetPos.y = 0.5f;

    bool isAtTarget = (glm::distance(glm::vec3(camera.Position.x, 0.0f, camera.Position.z), 
                                     glm::vec3(targetPos.x, 0.0f, targetPos.z)) < 0.01f) && 
                      (std::abs(camera.Yaw - targetYaw) < 0.1f);

    if (isAtTarget) {
        camera.SetPose(glm::vec3(targetPos.x, camera.Position.y, targetPos.z), targetYaw);

        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) targetYaw -= 90.0f;
        else if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) targetYaw += 90.0f;
        else if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
            
            glm::vec3 forward = glm::vec3(cos(glm::radians(targetYaw)), 0.0f, sin(glm::radians(targetYaw)));
            forward = glm::round(forward) * 1.0f;

            glm::vec3 nextPos = targetPos;
            if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) nextPos += forward;
            if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) nextPos -= forward;

            if (!checkCollision(nextPos.x, nextPos.z)) {
                targetPos.x = nextPos.x;
                targetPos.z = nextPos.z;
            }
        }
    }

    // Interpolation
    float smoothSpeed = 10.0f * deltaTime;
    glm::vec3 newPos = glm::mix(camera.Position, targetPos, smoothSpeed);
    float newYaw = glm::mix(camera.Yaw, targetYaw, smoothSpeed);
    camera.SetPose(newPos, newYaw);
}

void Level1::update(float deltaTime) {
    const auto& grid = maze->getGrid();
    int playerGridX = static_cast<int>(camera.Position.x);
    int playerGridZ = static_cast<int>(camera.Position.z);

    if (playerGridZ >= 0 && playerGridZ < grid.size() && 
        playerGridX >= 0 && playerGridX < grid[0].size()) {
        
        if (grid[playerGridZ][playerGridX] == 2) {
            std::cout << "Level 1 done!" << std::endl;
            
            // TODO: Transition to Level 2
            glfwSetWindowShouldClose(glfwGetCurrentContext(), true);
        }
    }
}

void Level1::render() {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    shader->use();

    // --- Matrix Setup ---
    int width, height;
    glfwGetWindowSize(glfwGetCurrentContext(), &width, &height);
    
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.1f, 100.0f);
    shader->setMat4("projection", projection);

    glm::mat4 view = camera.GetViewMatrix();
    shader->setMat4("view", view);

    glm::mat4 model = glm::mat4(1.0f);
    shader->setMat4("model", model);

    // 1. Black fill for walls
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(1.0f, 1.0f);
    shader->setVec4("color", 0.0f, 0.0f, 0.0f, 1.0f);
    mazeMesh->draw();
    glDisable(GL_POLYGON_OFFSET_FILL);

    // 2. Green wireframe for walls
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    shader->setVec4("color", 0.0f, 1.0f, 0.0f, 1.0f);
    mazeMesh->draw();
}