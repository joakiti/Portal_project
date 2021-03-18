//
// Created by mikke on 05-12-2020.
//
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <shader.h>
#include <random>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_access.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_inverse.inl>
#include "Portal.h"

# define M_PI           3.14159265358979323846  /* pi */


using namespace glm;

void Portal::Draw(Shader *shader, Shader *borderShader, mat4 view, mat4 proj) {
    DrawWithoutBorder(shader, view, proj);
    DrawBorder(borderShader, view, proj);
}

void Portal::DrawPerpendicular(Shader *shader, Shader *borderShader, mat4 view, mat4 proj) {
    shader->use();

    mat4 cheatLocal = mat4(1.0f);
    cheatLocal = translate(cheatLocal, position);
    cheatLocal = glm::rotate(cheatLocal, -glm::radians(c->Yaw), glm::vec3(0.f, 1., 0.0f));
    cheatLocal = glm::rotate(cheatLocal, glm::radians(90.f), vec3(0, 1.f, 0));

    shader->setMat4("projection", proj);
    shader->setMat4("model", cheatLocal);
    shader->setMat4("view", view);

    glBindVertexArray(this->VAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    borderShader->use();
    borderShader->setMat4("projection", proj);
    borderShader->setMat4("model", cheatLocal);
    borderShader->setMat4("view", view);
    if (idx == 1) {
        borderShader->setVec3("color", vec3(0, 0, 1.f));
    } else {
        borderShader->setVec3("color", vec3(1, 0, 0));
    }
    glBindVertexArray(VAOBorder);
    glDrawElements(GL_TRIANGLES, 24, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void Portal::DrawWithoutBorder(Shader *shader, mat4 view, mat4 proj) {
    shader->use();

    shader->setMat4("projection", proj);
    shader->setMat4("model", localToWorld);
    shader->setMat4("view", view);

    glBindVertexArray(this->VAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void Portal::DrawBorder(Shader *borderShader, mat4 view, mat4 proj) {
    borderShader->use();
    borderShader->setMat4("projection", proj);
    borderShader->setMat4("model", localToWorld);
    borderShader->setMat4("view", view);
    if (idx == 1) {
        borderShader->setVec3("color", vec3(0, 0, 1.f));
    } else {
        borderShader->setVec3("color", vec3(1, 0, 0));
    }
    glBindVertexArray(VAOBorder);
    glDrawElements(GL_TRIANGLES, 24, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

mat4 Portal::calculateView(mat4 view) {
    mat4 worldToView = view;
    mat4 otherPortalWorldToLocal = inverse(this->otherPortal->localToWorld);
    mat4 finalView = worldToView
                     * this->localToWorld
                     * glm::rotate(glm::mat4(1.0), glm::radians(180.0f), glm::vec3(0.0, 1.0, 0.0))
                     * otherPortalWorldToLocal;
    return finalView;
}
mat4 Portal::calculateViewNoRotation(mat4 view) {
    mat4 worldToView = view;
    mat4 otherPortalWorldToLocal = inverse(this->otherPortal->localToWorld);
    mat4 finalView = worldToView
                     * this->localToWorld
                     * otherPortalWorldToLocal;
    return finalView;
}

Portal::Portal(vec3 position, vec3 normal, Portal *otherPortal, GLuint fbo, GLuint tex, int idx, Camera *c) : Portal(position,
                                                                                                          normal,
                                                                                                          otherPortal,
                                                                                                          c) {
    this->idx = idx;
    texture = tex;
    framebuffer = fbo;
}

Portal::Portal(vec3 position, vec3 normal, Portal *otherPortal, Camera *c) : otherPortal(otherPortal), position(position),
                                                                  normal(normal), c(c) {
    if (otherPortal != nullptr) {
        /**
         * Check if the other portal is pointing to another portal than us
         */
        if (otherPortal->otherPortal != nullptr) {
            delete otherPortal->otherPortal;
        }
        otherPortal->otherPortal = this;
    }
    localToWorld = translate(mat4(1.0f), position);
    localToWorld = glm::rotate(localToWorld, -glm::radians(c->Yaw), glm::vec3(0.f, 1., 0.0f));
    localToWorld = glm::rotate(localToWorld, glm::radians(90.f), vec3(0, 1.f, 0));
    float portalVertices[] = {
            -1.f, 1.f, 0.0f, 0, 1.f,
            -1.f, -1.f, 0.0f, 0, 0,
            1.f, -1.f, 0.0f, 1.f, 0,
            1.f, 1.f, 0.0f, 1.f, 1.f,
    };

    GLuint indices[] = {
            3, 1, 0,
            3, 2, 1,
    };
    glGenVertexArrays(1, &this->VAO);
    glGenBuffers(1, &this->VBO);
    glGenBuffers(1, &this->EBO);
    glBindVertexArray(this->VAO);
    glBindBuffer(GL_ARRAY_BUFFER, this->VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(portalVertices), portalVertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    parsePositionAttribute();
    parseTextureAttribute();
    float borderVertices[] = {
            -1.1f, 1.1f, 0.0f, //Left top 0
            -1.1f, 1.f, 0.0f, //Slightly underneath left top 1
            -1.f, 1.1f, 0.0f, //Slightly to the right of left top 2
            1.1f, 1.1f, 0.0f, //Right top 3
            1.1f, 1.f, 0.0f, //Sligtly underneath right top 4
            1.f, 1.1f, 0.0f, //slightly to the left of right top 5
            -1.1f, -1.1f, 0.0f, //Left bottom 6
            -1.1f, -1.f, 0.0f, //Slightly above left bottom 7
            -1.f, -1.1f, 0.0f, //Slightly to the right of left bottom 8
            1.1f, -1.1f, 0.0f, //Right bottom 9
            1.f, -1.1f, 0.0f, //Slightly to the left of right bottom 10
            1.1f, -1.f, 0.0f, //Slightly ablove right bottom 11
    };
    GLuint borderIndices[] = {
            //TOP BORDER
            0, 3, 1,
            3, 4, 1,
            //LEFT BORDER
            6, 8, 0,
            0, 2, 8,
            //BOTTOM BORDER
            6, 9, 11,
            11, 6, 7,
            //RIGHT BORDER
            9, 10, 3,
            10, 3, 5
    };
    glGenVertexArrays(1, &this->VAOBorder);
    glGenBuffers(1, &this->VBOBorder);
    glGenBuffers(1, &this->EBOBorder);
    glBindVertexArray(this->VAOBorder);
    glBindBuffer(GL_ARRAY_BUFFER, this->VBOBorder);
    glBufferData(GL_ARRAY_BUFFER, sizeof(borderVertices), borderVertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->EBOBorder);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(borderIndices), borderIndices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *) 0);
    glBindVertexArray(0);
}

void Portal::parseTextureAttribute() const {// texture coord attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *) (3 * sizeof(float)));
    glEnableVertexAttribArray(1);
}

void Portal::parsePositionAttribute() const {// position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *) 0);
    glEnableVertexAttribArray(0);
}

mat4 Portal::clippedProjMat(mat4 view, mat4 proj) {
    /**
     * Based on https://github.com/ThomasRinsma/opengl-game-test/blob/8363bbfcce30acc458b8faacc54c199279092f81/src/sceneobject/portal.cc
     * From: http://www.terathon.com/code/oblique.html
     */
    float dist = glm::length(position);
    glm::vec4 clipPlane(glm::fquat(1.0f, 0.0f, 0.0f, 0.0f) * glm::vec3(0.0f, 0.0f, -1.0f), dist);

    clipPlane = glm::inverse(glm::transpose(view)) * clipPlane;

    if (clipPlane.w > 0.0f)
        return proj;

    glm::vec4 q = glm::inverse(proj) * glm::vec4(
            glm::sign(clipPlane.x),
            glm::sign(clipPlane.y),
            1.0f,
            1.0f
    );

    glm::vec4 c = clipPlane * (2.0f / (glm::dot(clipPlane, q)));

    glm::mat4 newProj = proj;
    // third row = clip plane - fourth row
    newProj = glm::row(newProj, 2, c - glm::row(newProj, 3));

    return newProj;
}


