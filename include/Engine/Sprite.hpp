#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>
#include "Engine/Shader.hpp"

class Sprite {
public:
    glm::vec3 Position;
    glm::vec2 Scale;
    GLuint TextureID;

    Sprite(glm::vec3 position, glm::vec2 scale, GLuint textureID)
        : Position(position), Scale(scale), TextureID(textureID) {
        initQuad();
    }

    void Draw(Shader* shader, const glm::vec3& cameraPos, const glm::mat4& view, const glm::mat4& projection) const {
        if (s_VAO == 0) {
            initQuad();
        }

        glm::vec3 toCamera = cameraPos - Position;
        toCamera.y = 0.0f;

        float angle = 0.0f;
        if (glm::dot(toCamera, toCamera) > 0.0001f) {
            angle = std::atan2(toCamera.x, toCamera.z);
        }

        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, Position);
        model = glm::rotate(model, angle, glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::scale(model, glm::vec3(Scale.x, Scale.y, 1.0f));

        shader->use();
        shader->setMat4("model", model);
        shader->setMat4("view", view);
        shader->setMat4("projection", projection);
        shader->setInt("spriteTexture", 0);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, TextureID);

        glBindVertexArray(s_VAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);
    }

    static void CleanupSharedMesh() {
        if (s_VAO != 0) {
            glDeleteVertexArrays(1, &s_VAO);
            s_VAO = 0;
        }
        if (s_VBO != 0) {
            glDeleteBuffers(1, &s_VBO);
            s_VBO = 0;
        }
    }

private:
    static inline GLuint s_VAO = 0;
    static inline GLuint s_VBO = 0;

    static void initQuad() {
        if (s_VAO != 0) return;

        float vertices[] = {
            -0.5f, 0.0f, 0.0f, 0.0f, 0.0f,
             0.5f, 0.0f, 0.0f, 1.0f, 0.0f,
             0.5f, 1.0f, 0.0f, 1.0f, 1.0f,

            -0.5f, 0.0f, 0.0f, 0.0f, 0.0f,
             0.5f, 1.0f, 0.0f, 1.0f, 1.0f,
            -0.5f, 1.0f, 0.0f, 0.0f, 1.0f
        };

        glGenVertexArrays(1, &s_VAO);
        glGenBuffers(1, &s_VBO);

        glBindVertexArray(s_VAO);
        glBindBuffer(GL_ARRAY_BUFFER, s_VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);

        glBindVertexArray(0);
    }
};