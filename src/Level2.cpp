#include "Levels/Level2.hpp"
#include "Engine/stb_image.h"
#include <iostream>

Level2::Level2(Engine* engine) 
    : gameEngine(engine), shader(nullptr), maze(nullptr), mazeMesh(nullptr), 
      audio(nullptr), compassShader(nullptr),
      firstMouse(true), lastX(635.0f), lastY(360.0f),
      noclip(false), fKeyPressed(false), compassVAO(0), compassVBO(0) {}

Level2::~Level2() {
    if (shader) delete shader;
    if (maze) delete maze;
    if (mazeMesh) delete mazeMesh;
    if (compassShader) delete compassShader;
    if (compassVAO) glDeleteVertexArrays(1, &compassVAO);
    if (compassVBO) glDeleteBuffers(1, &compassVBO);
    if (audio) delete audio; // Очищаем звук; деструктор Audio остановит музыку сам
    
    // Очистка всех загруженных текстур
    for (GLuint tex : textures) {
        if (tex != 0) glDeleteTextures(1, &tex);
    }
}

GLuint Level2::loadTexture(const char* path) {
    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    // Пиксельная фильтрация в стиле DOS / Wolfenstein 3D сохраняет хрустящие пиксели
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(true);
    unsigned char *data = stbi_load(path, &width, &height, &nrChannels, 0);
    if (data) {
        GLenum format = GL_RGB;
        if (nrChannels == 1) format = GL_RED;
        else if (nrChannels == 4) format = GL_RGBA;

        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    } else {
        std::cout << "Failed to load texture at path: " << path << std::endl;
    }
    stbi_image_free(data);

    return textureID;
}

void Level2::init() {
    camera = Camera(glm::vec3(1.5f, 0.5f, 1.5f));
    camera.Pitch = 0.0f; // Фиксация горизонтального взгляда при спавне

    shader = new Shader("shaders/level2.vert", "shaders/level2.frag");

    maze = new MazeGenerator(25, 25);
    maze->generate();

    mazeMesh = new MazeMesh();
    mazeMesh->buildTextured(maze->getGrid());

    // Загрузка 4 текстур из папки textures/ (строгое соответствие индексам батчей)
    textures.push_back(loadTexture("textures/Level2/wall1.png")); // texIndex 0
    textures.push_back(loadTexture("textures/Level2/wall2.png")); // texIndex 1
    textures.push_back(loadTexture("textures/Level2/wall3.png")); // texIndex 2
    textures.push_back(loadTexture("textures/Level2/exit.png"));  // texIndex 3

    shader->use();
    shader->setInt("texWall1", 0);
    shader->setInt("texWall2", 1);
    shader->setInt("texWall3", 2);
    shader->setInt("texExit", 3);

    compassShader = new Shader("shaders/compass.vert", "shaders/compass.frag");

    float compassVertices[] = {
         0.0f,  15.0f, // Острие (указывает вверх)
        -10.0f, -10.0f, // Левый нижний угол
         10.0f, -10.0f  // Правый нижний угол
    };

    glGenVertexArrays(1, &compassVAO);
    glGenBuffers(1, &compassVBO);
    
    glBindVertexArray(compassVAO);
    glBindBuffer(GL_ARRAY_BUFFER, compassVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(compassVertices), compassVertices, GL_STATIC_DRAW);
    
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    glBindVertexArray(0);

    // --- Запуск звука ---
    audio = new Audio(1);
    audio->playMusic("sounds/Level2/Music.mp3");
}

bool Level2::checkCollision(float x, float z) {
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

void Level2::processInput(GLFWwindow* window, float deltaTime) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }

    if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS) {
        if (!fKeyPressed) {
            noclip = !noclip;
            fKeyPressed = true;
            std::cout << "Level 2 Noclip: " << (noclip ? "ON" : "OFF") << std::endl;
        }
    } else {
        fKeyPressed = false;
    }

    double xposIn, yposIn;
    glfwGetCursorPos(window, &xposIn, &yposIn);
    float xpos = static_cast<float>(xposIn);

    if (firstMouse) {
        lastX = xpos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    lastX = xpos;

    // Свободный обзор по горизонтали, блокировка по вертикали
    camera.ProcessMouseMovement(xoffset, 0.0f);
    camera.Pitch = 0.0f;

    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)  camera.ProcessMouseMovement(-150.0f * deltaTime, 0.0f);
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) camera.ProcessMouseMovement(150.0f * deltaTime, 0.0f);
    camera.Pitch = 0.0f;

    // Плавное перемещение
    float velocity = camera.MovementSpeed * deltaTime * (noclip ? 2.0f : 1.0f);
    glm::vec3 moveDir(0.0f);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) moveDir += camera.Front;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) moveDir -= camera.Front;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) moveDir -= camera.Right;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) moveDir += camera.Right;

    static float stepTimer = 0.4f; // Таймер для звука шагов

    if (glm::length(moveDir) > 0.0f) {
        if (noclip) {
            camera.Position += glm::normalize(moveDir) * velocity;
        } else {
            // Звук шагов Эры 2
            stepTimer += deltaTime;
            if (stepTimer > 0.4f) {
                audio->playSound("sounds/Level2/Step.wav");
                stepTimer = 0.0f;
            }

            // Векторное скольжение по стенам
            moveDir.y = 0.0f;
            moveDir = glm::normalize(moveDir) * velocity;

            if (!checkCollision(camera.Position.x + moveDir.x, camera.Position.z)) {
                camera.Position.x += moveDir.x;
            }
            if (!checkCollision(camera.Position.x, camera.Position.z + moveDir.z)) {
                camera.Position.z += moveDir.z;
            }
        }
    } else {
        // Чтобы шаг зазвучал сразу же при начале нового движения
        stepTimer = 0.4f;
    }
}

void Level2::update(float deltaTime) {
    (void)deltaTime;
    const auto& grid = maze->getGrid();
    int playerGridX = static_cast<int>(camera.Position.x);
    int playerGridZ = static_cast<int>(camera.Position.z);

    if (playerGridZ >= 0 && playerGridZ < static_cast<int>(grid.size()) && 
        playerGridX >= 0 && playerGridX < static_cast<int>(grid[0].size())) {
        
        if (grid[playerGridZ][playerGridX] == 2) {
            std::cout << "=================================" << std::endl;
            std::cout << "УРОВЕНЬ 2 ПРОЙДЕН! Вы прошли Эру 2!" << std::endl;
            std::cout << "=================================" << std::endl;
            // Здесь в будущем будет инициализация Level3
            glfwSetWindowShouldClose(glfwGetCurrentContext(), true);
        }
    }
}

void Level2::render() {
    int width, height;
    glfwGetWindowSize(glfwGetCurrentContext(), &width, &height);

    glEnable(GL_SCISSOR_TEST);

    // Отрисовка потолка
    glScissor(0, height / 2, width, height / 2);
    glClearColor(0.2f, 0.2f, 0.25f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Отрисовка пола
    glScissor(0, 0, width, height / 2);
    glClearColor(0.35f, 0.35f, 0.35f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glDisable(GL_SCISSOR_TEST);

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glEnable(GL_DEPTH_TEST);

    shader->use();

    glm::mat4 projection = glm::perspective(glm::radians(60.0f), (float)width / (float)height, 0.1f, 100.0f);
    shader->setMat4("projection", projection);

    glm::mat4 view = camera.GetViewMatrix();
    shader->setMat4("view", view);

    glm::mat4 model = glm::mat4(1.0f);
    shader->setMat4("model", model);

    // Передаем массив загруженных текстур в меш
    mazeMesh->drawTextured(textures);

    // --- Отрисовка компаса (HUD) ---
    glDisable(GL_DEPTH_TEST); // Отключаем тест глубины, чтобы компас был поверх стен

    compassShader->use();
    // Ортографическая проекция по размерам окна
    glm::mat4 orthoProj = glm::ortho(0.0f, (float)width, 0.0f, (float)height);
    compassShader->setMat4("projection", orthoProj);

    // 1. Получаем мировые координаты центра клетки выхода
    float exitX = maze->getExitX() * 1.0f + 0.5f;
    float exitZ = maze->getExitY() * 1.0f + 0.5f;
    
    // 2. Рассчитываем вектор к выходу и угол
    float dirX = exitX - camera.Position.x;
    float dirZ = exitZ - camera.Position.z;
    float angleToExit = std::atan2(dirZ, dirX);
    float cameraAngle = glm::radians(camera.Yaw);
    
    // Угол поворота стрелки
    float rotation = cameraAngle - angleToExit;

    // 3. Формируем Model матрицу (размещение в правом нижнем углу)
    glm::mat4 modelCompass = glm::mat4(1.0f);
    modelCompass = glm::translate(modelCompass, glm::vec3(width - 70.0f, 70.0f, 0.0f));
    modelCompass = glm::scale(modelCompass, glm::vec3(1.5f, 1.5f, 1.0f)); 
    modelCompass = glm::rotate(modelCompass, rotation, glm::vec3(0.0f, 0.0f, 1.0f)); 
    
    compassShader->setMat4("model", modelCompass);

    // Отрисовка компаса
    glBindVertexArray(compassVAO);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glBindVertexArray(0);

    glEnable(GL_DEPTH_TEST); // Включаем обратно
}