#pragma once
#include <vector>
#include <string>
#include <glad/glad.h>
#include "Level.hpp"
#include "Engine/Engine.hpp"
#include "Engine/MazeGenerator.hpp"
#include "Engine/PrimitiveMesh.hpp"
#include "Engine/Shader.hpp"
#include "Engine/Camera.hpp"
#include "Engine/Audio.hpp"

struct OfficeLamp {
    glm::vec3 position;
    float timer;
    float nextFlicker;
    float intensity;
    bool soundPlayed;
};

struct SoundTrigger {
    glm::vec3 position;
    float radius;
    std::string soundPath;
    bool triggered;
};

class Level4 : public Level {
public:
    Level4(Engine* engine);
    ~Level4();

    void init() override;
    void processInput(GLFWwindow* window, float deltaTime) override;
    void update(float deltaTime) override;
    void render() override;

    std::string getHudHint() const;

private:
    Engine* gameEngine;
    Camera camera;
    Shader* shader;
    Shader* compassShader;
    MazeGenerator* maze;
    Audio* audio;

    LitMesh wallBatches[4];
    LitMesh floorCeilMesh;
    LitMesh lampPanelMesh;
    LitMesh fanBaseMesh;
    LitMesh fanBladeMesh;
    LitMesh switchBoxMesh;
    LitMesh switchLeverMesh;

    GLuint compassVAO;
    GLuint compassVBO;

    std::vector<GLuint> wallTextures;
    GLuint floorTexture;
    GLuint ceilTexture;

    std::vector<OfficeLamp> officeLamps;
    std::vector<SoundTrigger> soundTriggers;
    std::vector<glm::vec3> fanPositions;

    glm::vec3 breakerPos;
    float breakerYaw;
    bool powerRestored;
    float leverAngle;

    bool flashlightOn;
    bool fKeyPressed;
    bool eKeyPressed;
    bool nKeyPressed;
    bool noclip;

    bool firstMouse;
    float lastX;
    float lastY;
    float elapsedTime;

    void setupLampsAndTriggers();
    void playSoundSafe(const std::string& path);
    bool checkCollision(float x, float z);
    GLuint loadTexture(const char* path);
};