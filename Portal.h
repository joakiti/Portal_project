//
// Created by mikke on 05-12-2020.
//

#include <glm/gtc/quaternion.hpp>  // for glm::fquat
#include "camera.h"

#ifndef ITU_GRAPHICS_PROGRAMMING_PORTAL_H
#define ITU_GRAPHICS_PROGRAMMING_PORTAL_H

using namespace glm;

class Portal {
public:
    Portal(vec3 position, vec3 normal, Portal *otherPortal, Camera *c = nullptr);

    Portal(vec3 position, vec3 normal, Portal *otherPortal, GLuint fbo, GLuint tex, int idx, Camera *c);

    GLuint texture, framebuffer;
    mat4 localToWorld;
    vec3 position, normal;
    int idx;
    Portal *otherPortal;
    Camera *c;

    mat4 calculateView(mat4 view);
    mat4 calculateViewNoRotation(mat4 view);
    mat4 clippedProjMat(mat4 view, mat4 proj);

    void DrawWithoutBorder(Shader *shader, mat4 view, mat4 proj);
    void DrawBorder(Shader *borderShader, mat4 view, mat4 proj);
    void DrawPerpendicular(Shader *shader, Shader *borderShader, mat4 view, mat4 proj);
    void Draw(Shader *shader, Shader *borderShader, mat4 view, mat4 proj);

private:
    GLuint VAO, VBO, EBO;
    GLuint VAOBorder, VBOBorder, EBOBorder;

    void parsePositionAttribute() const;

    void parseTextureAttribute() const;

};


#endif //ITU_GRAPHICS_PROGRAMMING_PORTAL_H
