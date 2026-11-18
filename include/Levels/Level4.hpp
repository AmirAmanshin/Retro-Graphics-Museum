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
#include "Engine/Sprite.hpp"

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

enum class MonsterState {
    PATROL,
    CHASE
};

struct StalkerMonster {
    glm::vec3 pos;
    glm::vec3 targetPos;
    bool hasTarget;
    MonsterState state;
    float patrolSpeed;
    float chaseSpeed;
    float loseSightTimer;
    float animTimer;
    int animFrame;
    float groanTimer;
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
    Shader* spriteShader;
    Shader* compassShader;
    Shader* ps1PostShader;
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
    GLuint pdaQuadVAO;
    GLuint pdaQuadVBO;

    GLuint ps1FBO;
    GLuint ps1ColorTex;
    GLuint ps1DepthRBO;
    GLuint screenQuadVAO;
    GLuint screenQuadVBO;
    static constexpr int PS1_WIDTH = 240;
    static constexpr int PS1_HEIGHT = 180;

    std::vector<GLuint> wallTextures;
    GLuint floorTexture;
    GLuint ceilTexture;
    GLuint monsterTex1;
    GLuint monsterTex2;

    std::vector<OfficeLamp> officeLamps;
    std::vector<SoundTrigger> soundTriggers;
    std::vector<glm::vec3> fanPositions;

    StalkerMonster monster;
    Sprite* monsterSprite;

    glm::vec3 breakerPos;
    float breakerYaw;
    bool powerRestored;
    float leverAngle;

    bool pdaHeld;
    float pdaOpenProgress;

    float stamina;
    float maxStamina;
    bool isSprinting;
    bool isExhausted;
    float catchFlashTimer;

    bool flashlightOn;
    bool fKeyPressed;
    bool eKeyPressed;
    bool nKeyPressed;
    bool noclip;

    bool firstMouse;
    float lastX;
    float lastY;
    float elapsedTime;

    void initPs1Framebuffer();
    void setupLampsAndTriggers();
    void spawnMonster();
    void updateMonsterAI(float deltaTime);
    bool hasLineOfSight(const glm::vec3& from, const glm::vec3& to) const;
    glm::ivec2 getNextStepBFS(glm::ivec2 start, glm::ivec2 goal) const;

    void renderPDA(int screenW, int screenH);
    void drawPdaRect(float x, float y, float w, float h, const glm::vec4& color, const glm::mat4& proj);

    void playSoundSafe(const std::string& path);
    bool checkCollision(float x, float z);
    GLuint loadTexture(const char* path, bool clampToEdge = false, bool allowMissing = false);
    GLuint createFallbackMonsterTexture();
};