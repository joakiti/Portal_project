#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>  // for glm::fquat
#include <Shader.h>
#include <iostream>
#include "camera.h"
#include "Portal.h"

#define STB_IMAGE_IMPLEMENTATION
#define M_PI           3.14159265358979323846  /* pi */

#include <stb_image.h>

struct CameraModel {
    glm::mat4 model;
    glm::mat4 view;
};

void framebuffer_size_callback(GLFWwindow *window, int width, int height);

int portal_intersection(glm::vec4 la, glm::vec4 lb, Portal portal);

void processInput(GLFWwindow *window);

void render(mat4 view, mat4 projection, vec3 cubePositions[], unsigned int BoxesVAO, mat4 globalModel = mat4(1.0f));

void mouse_button_callback(GLFWwindow *window, int button, int action, int mods);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

unsigned int floorTexture;
unsigned int floorVAO;
// global variables used for control
// ---------------------------------
float lastX = (float) SCR_WIDTH / 2.0;
float lastY = (float) SCR_HEIGHT / 2.0;
float deltaTime;
bool isPaused = false; // stop camera movement when GUI is open
bool stencilBuffer = false;
bool debug = false;
bool showBluePortalsCamera = false;

Camera camera(vec3(0.0f, 0.0f, 1.0f));

vec3 positiveZ = vec3(0.0f, 0.0f, 1.0f);

Shader *ourShader;
Shader *portalShader;
Shader *cameraShader;
Shader *floorShader;

int portalIndex = -1;
bool didTeleport[] = {false, false};
// timing
float lastFrame = 0.0f;
bool portalDrawn = false;

Portal *portals[2];
CameraModel *virtualCameras[2];

void mouse_callback(GLFWwindow *window, double xpos, double ypos);

unsigned int loadTexture(int &width, int &height, const char *path);

unsigned int loadTexture(char const *path);

bool noPortalDrawn();

void generateFrameBufferTexture(unsigned int &rbo, unsigned int &framebuffer, unsigned int &texture);

unsigned char *loadTexture(unsigned int &texture2, int &width, int &height, int &nrChannels, char *path);

void loadBoxTextures(unsigned int &texture1, unsigned int &texture2);

void FBOApproach(mat4 projection, vec3 cubePositions[], unsigned int VAO);

void stencilApproach(mat4 projection, vec3 cubePositions[], unsigned int VAO);

void generateTextureForPortals(const mat4 projection, mat4 view, vec3 *cubePositions, unsigned int fbo, Portal *portal,
                               unsigned int VAO, int depth);

void drawDebuggingCameras(unsigned int VAO, mat4 &projection);

void disableWritingToDepthAndColor();

void enableWritingToDepthAndColor();


void updateDebugCameraPositions();

mat4 prevView;

int main() {
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    GLFWwindow *window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);


    // build and compile our shader zprogram
    // ------------------------------------
    ourShader = new Shader("shaders/vert.shader", "shaders/scene.frag");
    portalShader = new Shader("shaders/portal.vert", "shaders/portal.frag");
    cameraShader = new Shader("shaders/camera.vert", "shaders/camera.frag");
    floorShader = new Shader("shaders/floor.vert", "shaders/floor.frag");
    // set up vertex data (and buffer(s)) and configure vertex attributess
    // ------------------------------------------------------------------
    float vertices[] = {
            -0.5f, -0.5f, -0.5f, 0.0f, 0.0f,
            0.5f, -0.5f, -0.5f, 1.0f, 0.0f,
            0.5f, 0.5f, -0.5f, 1.0f, 1.0f,
            0.5f, 0.5f, -0.5f, 1.0f, 1.0f,
            -0.5f, 0.5f, -0.5f, 0.0f, 1.0f,
            -0.5f, -0.5f, -0.5f, 0.0f, 0.0f,

            -0.5f, -0.5f, 0.5f, 0.0f, 0.0f,
            0.5f, -0.5f, 0.5f, 1.0f, 0.0f,
            0.5f, 0.5f, 0.5f, 1.0f, 1.0f,
            0.5f, 0.5f, 0.5f, 1.0f, 1.0f,
            -0.5f, 0.5f, 0.5f, 0.0f, 1.0f,
            -0.5f, -0.5f, 0.5f, 0.0f, 0.0f,

            -0.5f, 0.5f, 0.5f, 1.0f, 0.0f,
            -0.5f, 0.5f, -0.5f, 1.0f, 1.0f,
            -0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
            -0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
            -0.5f, -0.5f, 0.5f, 0.0f, 0.0f,
            -0.5f, 0.5f, 0.5f, 1.0f, 0.0f,

            0.5f, 0.5f, 0.5f, 1.0f, 0.0f,
            0.5f, 0.5f, -0.5f, 1.0f, 1.0f,
            0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
            0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
            0.5f, -0.5f, 0.5f, 0.0f, 0.0f,
            0.5f, 0.5f, 0.5f, 1.0f, 0.0f,

            -0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
            0.5f, -0.5f, -0.5f, 1.0f, 1.0f,
            0.5f, -0.5f, 0.5f, 1.0f, 0.0f,
            0.5f, -0.5f, 0.5f, 1.0f, 0.0f,
            -0.5f, -0.5f, 0.5f, 0.0f, 0.0f,
            -0.5f, -0.5f, -0.5f, 0.0f, 1.0f,

            -0.5f, 0.5f, -0.5f, 0.0f, 1.0f,
            0.5f, 0.5f, -0.5f, 1.0f, 1.0f,
            0.5f, 0.5f, 0.5f, 1.0f, 0.0f,
            0.5f, 0.5f, 0.5f, 1.0f, 0.0f,
            -0.5f, 0.5f, 0.5f, 0.0f, 0.0f,
            -0.5f, 0.5f, -0.5f, 0.0f, 1.0f
    };
    // world space positions of our cubes
    vec3 cubePositions[] = {
            vec3(0.0f, 0.0f, 0.0f),
            vec3(2.0f, 5.0f, -15.0f),
            vec3(-1.5f, -2.2f, -2.5f),
            vec3(-3.8f, -2.0f, -12.3f),
            vec3(2.4f, -0.4f, -3.5f),
            vec3(-1.7f, 3.0f, -7.5f),
            vec3(1.3f, -2.0f, -2.5f),
            vec3(1.5f, 2.0f, -2.5f),
            vec3(1.5f, 0.2f, -1.5f),
            vec3(-1.3f, 1.0f, -1.5f)
    };
    unsigned int VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *) 0);
    glEnableVertexAttribArray(0);
    // texture coord attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *) (3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    unsigned int woodTexture;
    unsigned int smileyTexture;
    loadBoxTextures(woodTexture, smileyTexture);
    floorTexture = loadTexture(R"(C:\Users\mikke\Glitter\Glitter\Glitter\Sources\tiles.jpg)");

    float planeVertices[] = {
            // positions          // texture Coords (note we set these higher than 1 (together with GL_REPEAT as texture wrapping mode). this will cause the floor texture to repeat)
            5.0f, -0.5f, 5.0f, 2.0f, 0.0f,
            -5.0f, -0.5f, 5.0f, 0.0f, 0.0f,
            -5.0f, -0.5f, -5.0f, 0.0f, 2.0f,

            5.0f, -0.5f, 5.0f, 2.0f, 0.0f,
            -5.0f, -0.5f, -5.0f, 0.0f, 2.0f,
            5.0f, -0.5f, -5.0f, 2.0f, 2.0f
    };

    // plane VAO
    unsigned int planeVBO;
    glGenVertexArrays(1, &floorVAO);
    glGenBuffers(1, &planeVBO);
    glBindVertexArray(floorVAO);
    glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), &planeVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *) 0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *) (3 * sizeof(float)));
    glBindVertexArray(0);


    ourShader->use();
    ourShader->setInt("wood", 0);
    ourShader->setInt("smiley", 1);

    // tell opengl for each sampler to which texture unit it belongs to (only has to be done once)
    // -------------------------------------------------------------------------------------------

    // render loop
    // -----------
    mat4 projection = perspective(radians(45.0f), (float) SCR_WIDTH / (float) SCR_HEIGHT, 0.1f, 100.0f);
    ourShader->use();
    ourShader->setMat4("projection", projection);
    portalShader->use();
    portalShader->setMat4("projection", projection);
    portalShader->setInt("texture", 2);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, woodTexture);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, smileyTexture);
    int frame = 0;
    while (!glfwWindowShouldClose(window)) {
        processInput(window);
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        prevView = camera.GetViewMatrix();
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        if (!stencilBuffer) {
            FBOApproach(projection, cubePositions, VAO);
        } else {
            stencilApproach(projection, cubePositions, VAO);
        }
        if (debug) {
            updateDebugCameraPositions();
        }
        drawDebuggingCameras(VAO, projection);
        if (debug) {
            if (portals[0] != NULL && portals[1] != NULL) {
                for (auto portal : portals) {
                    float offsetFromPortal = glm::distance(camera.Position, portal->position);
                    if (frame % 500 == 0) {
                        printf("%f\n", offsetFromPortal);
                    }
                    frame++;
                    if (!didTeleport[portal->idx] && !didTeleport[portal->otherPortal->idx] &&
                        offsetFromPortal >= -0.5 &&
                        offsetFromPortal <= 0.5) {
                        mat4 positionOfVirtualCamera = inverse(portal->calculateViewNoRotation(camera.GetViewMatrix()));
                        vec3 newPos = vec3(positionOfVirtualCamera[3][0], positionOfVirtualCamera[3][1],
                                           positionOfVirtualCamera[3][2]);
                        vec3 newFront = vec3(positionOfVirtualCamera[0][2], positionOfVirtualCamera[1][2],
                                             positionOfVirtualCamera[2][2]);
                        camera.Position.x = newPos.x;
                        camera.Position.y = newPos.y;
                        camera.Position.z = newPos.z;
                        camera.Yaw += 180;
                        camera.updateCameraVectors();
                        didTeleport[portal->idx] = true;
                        didTeleport[portal->otherPortal->idx] = true;
                    }
                    else if (offsetFromPortal >= 1.5 || offsetFromPortal <= -1.5) {
                        didTeleport[portal->idx] = false;
                    }
                }
            }
        }
        /**
         * Draw cameras, even afte debugging has ended, so we can move to the cameras with our player camera and see them
         */
        /**
                  * /**
         * COLLISION DETECTION ATTEMPTS:
         *
     **/
        /**
           if (portals[0] != NULL && portals[1] != NULL) {
               for (int i = 0; i < 2; i++) {
                   glm::vec4 la = glm::inverse() * glm::vec4(0.0, 0.0, 0.0, 1.0);
                   glm::vec4 lb = glm::inverse(camera.GetViewMatrix()) * glm::vec4(0.0, 0.0, 0.0, 1.0);
                   if (portal_intersection(la, lb, *portals[i])) {
                       printf("CLOSE");
                       vec4 newPos = portals[i]->calculateView(camera.GetViewMatrix()) * vec4(camera.Position, 1.0);
                       camera.Position = vec3(newPos.x, newPos.y, newPos.z);
                   }
               }
           }
           **/
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

// optional: de-allocate all resources once they've outlived their purpose:
// ------------------------------------------------------------------------
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);

// glfw: terminate, clearing all previously allocated GLFW resources.
// ------------------------------------------------------------------
    glfwTerminate();

    return 0;
}

void updateDebugCameraPositions() {
    if (portals[0] != NULL && portals[1] != NULL) {
        for (auto portal : portals) {
            CameraModel *portalCamera;
            if (virtualCameras[portal->idx] != nullptr) {
                portalCamera = virtualCameras[portal->idx];
            } else {
                portalCamera = new CameraModel();
            }
            mat4 model = mat4(1.0f);
            model = translate(model, vec3(camera.Position));
            model = scale(model, vec3(0.5, 0.5, 0.5));
            portalCamera->model = model;
            virtualCameras[portal->idx] = portalCamera;
        }
    }
}

void drawDebuggingCameras(unsigned int VAO, mat4 &projection) {
    for (auto &portal : portals) {
        if (portal != nullptr && virtualCameras[portal->idx] != nullptr) {
            cameraShader->use();
            /**
             * First portals camera is blue, other is red.
            */
            vec3 color;
            if (portal->idx == 0) {
                color = vec3(0, 0, 1.f);
            } else {
                color = vec3(1, 0, 0);
            }
            cameraShader->setVec3("color", color);
            virtualCameras[portal->idx]->view = portal->calculateView(camera.GetViewMatrix());
            cameraShader->setMat4("view", virtualCameras[portal->idx]->view);
            cameraShader->setMat4("projection", projection);
            cameraShader->setMat4("model", virtualCameras[portal->idx]->model);
            glBindVertexArray(VAO);
            glDrawArrays(GL_TRIANGLES, 0, 36);
            cameraShader->setMat4("model", translate(scale(virtualCameras[portal->idx]->model, vec3(0.5, 0.5, 0.5)),
                                                     vec3(0.0f, 0.f, -1)));
            cameraShader->setVec3("color", color + vec3(0.5, 0.5, 0.5));
            glDrawArrays(GL_TRIANGLES, 0, 36);
            // camera/view localToWorld
        }
    }
}

void recursiveStencil(mat4 view, mat4 projection, vec3 cubePositions[], unsigned int VAO, int depth) {
    if (depth >= 2) {
        return;
    }
    if (portals[0] != NULL && portals[1] != NULL) {
        //Now, we have for
        for (int i = 0; i < 2; i++) {
            auto *p = portals[i];
            //Setup the stencil test, write 1's inside the portal frame
            glEnable(GL_STENCIL_TEST);
            glStencilMask(0xFF);
            glStencilFunc(GL_NEVER, depth + i + 1, 0xFF);
            glStencilOp(GL_REPLACE, GL_KEEP, GL_KEEP);
            //We are only interested in carving a hole, so remove depth test and depth color writing
            disableWritingToDepthAndColor();
            p->DrawWithoutBorder(ourShader, view, projection);
            //Then, enable the depth test, to get a correct rendering. Remember we only update depth
            //on the values that are actually "valid". But, yes, we clear the entire depth buffer
            enableWritingToDepthAndColor();
            glEnable(GL_DEPTH_TEST);
            glStencilFunc(GL_EQUAL, depth + i + 1, 0xFF);
            glStencilMask(0x00);
            mat4 newProj = perspective(radians(45.0f), (float) SCR_WIDTH / (float) SCR_HEIGHT,
                                       distance(camera.Position, p->position), 100.0f);
            render(p->calculateView(view), newProj, cubePositions, VAO, mat4(1.0f));
            //recursiveStencil(p->calculateView(view), projection, cubePositions, VAO, depth +1);
        }
        glDisable(GL_STENCIL_TEST);
        glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
        glClear(GL_DEPTH_BUFFER_BIT);
        for (auto portal : portals) {
            portal->DrawWithoutBorder(cameraShader, view, projection);
        }
    }
    glStencilFunc(GL_EQUAL, depth, 0xFF);
    glEnable(GL_DEPTH_TEST);
    enableWritingToDepthAndColor();
    render(view, projection, cubePositions, VAO, mat4(1.0f));
    for (auto portal : portals) {
        if (portal != NULL && portal->otherPortal == NULL) {
            cameraShader->use();
            cameraShader->setVec3("color", vec3(0, 0, 0));
            portal->Draw(cameraShader, cameraShader, view, projection);
        } else if (portal != NULL && portal->otherPortal != NULL) {
            portal->DrawBorder(cameraShader, view, projection);
        }
    }
}

/**
 * From
 * https://en.wikibooks.org/wiki/OpenGL_Programming/Mini-Portal
 * @param la
 * @param lb
 * @param portal
 * @return
 */
int portal_intersection(glm::vec4 la, glm::vec4 lb, Portal portal) {
    if (la != lb) {  // camera moved
        // Check for intersection with each of the portal's 2 front triangles
        for (int i = 0; i < 2; i++) {
            vec4 p0, p1, p2;
            if (i == 0) {
                // Portal coordinates in world view
                p0 = vec4(1.f, 1.f, 0.0f, 1.f);
                p1 = vec4(-1.f, -1.f, 0.0f, 1.f);
                p2 = vec4(-1.f, 1.f, 0.0f, 1.f);
            } else {
                p0 = vec4(1.f, 1.f, 0.0f, 1.f);
                p1 = vec4(-1.f, -1.f, 0.0f, 1.f);
                p2 = vec4(1.f, -1.f, 0.0f, 1.f);
            }
            // Solve line-plane intersection using parametric form
            glm::vec3 tuv =
                    glm::inverse(glm::mat3(glm::vec3(la.x - lb.x, la.y - lb.y, la.z - lb.z),
                                           glm::vec3(p1.x - p0.x, p1.y - p0.y, p1.z - p0.z),
                                           glm::vec3(p2.x - p0.x, p2.y - p0.y, p2.z - p0.z)))
                    * glm::vec3(la.x - p0.x, la.y - p0.y, la.z - p0.z);
            float t = tuv.x, u = tuv.y, v = tuv.z;

            // intersection with the plane
            if (t >= 0 - 1e-6 && t <= 1 + 1e-6) {
                // intersection with the triangle
                if (u >= 0 - 1e-6 && u <= 1 + 1e-6 && v >= 0 - 1e-6 && v <= 1 + 1e-6 && (u + v) <= 1 + 1e-6) {
                    return 1;
                }
            }
        }
    }
    return 0;
}

void enableWritingToDepthAndColor() {
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glDepthMask(GL_TRUE);
}

void disableWritingToDepthAndColor() {
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
    glDepthMask(GL_FALSE);
}

void stencilApproach(mat4 projection, vec3 cubePositions[], unsigned int VAO) {
    glEnable(GL_STENCIL_TEST);
    recursiveStencil(camera.GetViewMatrix(), projection, cubePositions, VAO, 0);
    glStencilMask(0xFF); // each bit is written to the stencil buffer as is
    glDisable(GL_STENCIL_TEST);
}


void FBOApproach(mat4 projection, vec3 cubePositions[], unsigned int VAO) {
    if (!showBluePortalsCamera) {
        for (auto &portal : portals) {
            if (portal != nullptr && portal->otherPortal != nullptr) {
                glBindFramebuffer(GL_FRAMEBUFFER, portal->framebuffer);
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                mat4 newProj = perspective(radians(45.0f), (float) SCR_WIDTH / (float) SCR_HEIGHT,
                                           distance(camera.Position, portal->position), 100.0f);
                generateTextureForPortals(newProj, portal->calculateView(camera.GetViewMatrix()), cubePositions,
                                          portal->framebuffer, portal, VAO, 0);
                glBindFramebuffer(GL_FRAMEBUFFER, 0); // back to default
            }
        }
        // second pass
        render(camera.GetViewMatrix(), projection, cubePositions, VAO, mat4(1.0f));
        for (auto &portal : portals) {
            glActiveTexture(GL_TEXTURE2);
            if (portal != nullptr) {
                glBindTexture(GL_TEXTURE_2D, portal->texture);
                if (debug) {
                    portal->DrawPerpendicular(portalShader, cameraShader, camera.GetViewMatrix(), projection);
                }
                else {
                    portal->Draw(portalShader, cameraShader, camera.GetViewMatrix(), projection);
                }
            }
        }
    } else
        /**
         * Render from point of view of blue portal
         */
    {
        mat4 finalView = portals[0]->calculateView(camera.GetViewMatrix());
        //mat4 projection = portal->clippedProjMat(finalView, projection);
        render(finalView, projection, cubePositions, VAO, mat4(1.0f));
        if (debug) {
            for (auto &portal : portals) {
                glBindFramebuffer(GL_FRAMEBUFFER, portal->framebuffer);
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                generateTextureForPortals(projection, portal->calculateView(camera.GetViewMatrix()), cubePositions,
                                          portal->framebuffer, portal, VAO, 0);
                glBindFramebuffer(GL_FRAMEBUFFER, 0); // back to default
                glActiveTexture(GL_TEXTURE2);
                if (portal != nullptr) {
                    glBindTexture(GL_TEXTURE_2D, portal->texture);
                    if (debug) {
                        portal->DrawPerpendicular(portalShader, cameraShader, camera.GetViewMatrix(), projection);
                    }
                    else {
                        portal->Draw(portalShader, cameraShader, camera.GetViewMatrix(), projection);
                    }
                }
            }
        }
    }
}

void generateTextureForPortals(const mat4 projection, mat4 view, vec3 *cubePositions, unsigned int fbo, Portal *portal,
                               unsigned int VAO, int depth) {
    if (depth > 1) {
        return;
    }
    //mat4 projection = portal->clippedProjMat(finalView, projection);
    render(view, projection, cubePositions, VAO, mat4(1.0f));
    if (debug) {
        /**
         *
        don't attempt to generate new fbos, this breaks it.
        unsigned int framebuffer, portalTexture, rbo;
        generateFrameBufferTexture(rbo, framebuffer, portalTexture);
         */
        glActiveTexture(GL_TEXTURE2);
        //glBindTexture(GL_TEXTURE_2D, portal->otherPortal->texture);
        glBindTexture(GL_TEXTURE_2D, portal->texture);
        portal->Draw(portalShader, cameraShader, view, projection);
    }
}

void loadBoxTextures(unsigned int &texture1, unsigned int &texture2) {// load and create a texture
// -------------------------
    int width, height, nrChannels;
    // ---------
    stbi_set_flip_vertically_on_load(true); // tell stb_image.h to flip loaded texture's on the y-axis.
    unsigned char *data = loadTexture(texture1, width, height, nrChannels,
                                      "resources/container.jpg");
    if (data) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    } else {
        std::cout << "Failed to load texture" << std::endl;
    }
    stbi_image_free(data);
    // texture 2
// ---------
    data = loadTexture(texture2, width, height, nrChannels,
                       "resources/awesomeface.png");
    if (data) {
        // note that the awesomeface.png has transparency and thus an alpha channel, so make sure to tell OpenGL the data type is of GL_RGBA
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    } else {
        std::cout << "Failed to load texture" << std::endl;
    }
    stbi_image_free(data);
}

unsigned char *loadTexture(unsigned int &texture2, int &width, int &height, int &nrChannels, char *path) {
    glGenTextures(1, &texture2);
    glBindTexture(GL_TEXTURE_2D, texture2);
    // set the texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // load image, create texture and generate mipmaps
    unsigned char *data = stbi_load(path, &width, &height, &nrChannels,
                                    0);
    return data;
}

void generateFrameBufferTexture(unsigned int &rbo, unsigned int &framebuffer, unsigned int &texture) {
    glGenFramebuffers(1, &framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    // generate texture
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    float borderColor[] = {0.8f, 0.2f, 0.0f, 1.0f};
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, SCR_WIDTH,
                          SCR_HEIGHT); // use a single renderbuffer object for both a depth AND stencil buffer.
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER,
                              rbo); // now actually attach it
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void render(mat4 view, mat4 projection, vec3 cubePositions[], unsigned int BoxesVAO, mat4 globalModel) {
    // render
    // ------
    // bind textures on corresponding texture units
    // activate shader
    ourShader->use();
    // camera/view localToWorld
    ourShader->setMat4("view", view);
    ourShader->setMat4("projection", projection);
    // render boxes
    glBindVertexArray(BoxesVAO);
    for (unsigned int i = 0; i < 10; i++) {
        // calculate the model matrix for each object and pass it to shader before drawing
        mat4 model = mat4(1.0f); // make sure to initialize matrix to identity matrix first
        model = translate(model, cubePositions[i]);
        float angle = 20.0f * i;
        model = rotate(model, radians(angle), vec3(1.0f, 0.3f, 0.5f));
        ourShader->setMat4("model", model * globalModel);
        glDrawArrays(GL_TRIANGLES, 0, 36);
    }
    floorShader->use();
    floorShader->setMat4("view", view);
    floorShader->setMat4("projection", projection);
    floorShader->setMat4("model", glm::mat4(1.0f) * globalModel);
    glBindVertexArray(floorVAO);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, floorTexture);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
    // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
    // -------------------------------------------------------------------------------
}

bool noPortalDrawn() { return portalIndex < 0; }

bool firstMouse = true;

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    // movement commands
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        camera.Position += vec3(0, 0.005, 0);
    if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
        camera.Position -= vec3(0, 0.005, 0);
    if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS)
        stencilBuffer = false;
    if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS)
        stencilBuffer = true;
    if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS)
        debug = true;
    if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS)
        debug = false;
    if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS)
        showBluePortalsCamera = true;
    if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS)
        showBluePortalsCamera = false;

}

void mouse_button_callback(GLFWwindow *window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        unsigned int framebuffer, portalTexture, rbo;
        generateFrameBufferTexture(rbo, framebuffer, portalTexture);
        vec3 position = camera.Position + camera.Front;
        vec3 direction = camera.Front;
        if (noPortalDrawn()) {
            portalIndex = 0;
            portals[portalIndex] = new Portal(vec3(position.x, 0.50, position.z), camera.Front, nullptr, framebuffer,
                                              portalTexture,
                                              portalIndex,
                                              &camera);
        } else {
            int nextPortal = (portalIndex + 1) % 2;
            portals[nextPortal] = new Portal(vec3(position.x, 0.50, position.z), camera.Front, portals[portalIndex],
                                             framebuffer,
                                             portalTexture,
                                             nextPortal,
                                             &camera);
            portalIndex = nextPortal;
        }
    }
}


void mouse_callback(GLFWwindow *window, double posX, double posY) {
    // camera rotation
    static bool firstMouse = true;
    if (firstMouse) {
        lastX = posX;
        lastY = posY;
        firstMouse = false;
    }

    float xoffset = posX - lastX;
    float yoffset = lastY - posY; // reversed since y-coordinates go from bottom to top

    lastX = posX;
    lastY = posY;
    camera.ProcessMouseMovement(xoffset, yoffset);
}

unsigned int loadTexture(char const *path) {
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data) {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    } else {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }
    return textureID;
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}