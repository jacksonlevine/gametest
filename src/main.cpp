
#include <iostream>
#include <atomic>
#include <string>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#define GLM_ENABLE_EXPERIMENTAL
// Define these only in *one* .cc file.
#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
// #define TINYGLTF_NOEXCEPTION // optional. disable exception handling.

#include "Camera.h"
#include "ModelLoader.h"
#include "Shader.h"
#include <glm/gtc/type_ptr.hpp>

#include "BasicGLTFShader.h"

GLFWwindow* WINDOW = nullptr;

std::atomic<int> SCREENWIDTH = 1280;
std::atomic<int> SCREENHEIGHT = 720;
std::atomic<float> FOV = 90.0f;
std::atomic<bool> CAMERA_PROJECTION_UPDATE_REQUESTED = true;

jl::Camera CAMERA;
jl::Shader* SHADER; //This must be initialized after the GL context, so it's a pointer

bool FIRSTMOUSE = true;
bool MOUSECAPTURED = false;

void frameBufferCallback(GLFWwindow* window, int width, int height)
{
    std::cout << std::to_string(width) << " " << std::to_string(height) << "\n";
    SCREENWIDTH.store(width); SCREENHEIGHT.store(height);
    CAMERA_PROJECTION_UPDATE_REQUESTED.store(true);
    glViewport(0, 0, width, height);
}

void cursorPosCallback(GLFWwindow* window, double xpos, double ypos)
{
    static double lastx = 0.0;
    static double lasty = 0.0;

    if(MOUSECAPTURED)
    {
        if(FIRSTMOUSE)
        {
            lastx = xpos;
            lasty = ypos;
            FIRSTMOUSE = false;
        }

        const double xOffset = (xpos - lastx) * 0.25;
        const double yOffset = (lasty - ypos) * 0.25;

        //std::cout << "Yaw: " << std::to_string(CAMERA.transform.yaw) << " Pitch: " << std::to_string(CAMERA.transform.pitch) << "\n";

        CAMERA.transform.yaw += static_cast<float>(xOffset);
        CAMERA.transform.pitch += static_cast<float>(yOffset);

        lastx = xpos;
        lasty = ypos;
    }
}

void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
    if(action)
    {
        if(button == GLFW_MOUSE_BUTTON_1)
        {
            glfwSetInputMode(WINDOW, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            MOUSECAPTURED = true;
        } else
        {
            glfwSetInputMode(WINDOW, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            MOUSECAPTURED = false;
            FIRSTMOUSE = true;
        }
    }
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if(action)
    {
        if(key == GLFW_KEY_W)
        {
            CAMERA.transform.position += CAMERA.transform.direction * 0.1f;
            CAMERA.updateWithYawPitch(CAMERA.transform.yaw, CAMERA.transform.pitch);
        }
        if(key == GLFW_KEY_S)
        {
            CAMERA.transform.position -= CAMERA.transform.direction * 0.1f;
            CAMERA.updateWithYawPitch(CAMERA.transform.yaw, CAMERA.transform.pitch);
        }
    }
}

int main() {

    if (glfwInit() != GLFW_TRUE)
    {
        std::cout << "Couldn't initialize glfw.\n";
    }

    WINDOW = glfwCreateWindow(SCREENWIDTH.load(), SCREENHEIGHT.load(), "Game", nullptr, nullptr);
    glfwMakeContextCurrent(WINDOW);
    glfwSwapInterval( 0 ); //Disable v-sync
    glfwSetFramebufferSizeCallback(WINDOW, frameBufferCallback);
    glfwSetCursorPosCallback(WINDOW, cursorPosCallback);
    glfwSetMouseButtonCallback(WINDOW, mouseButtonCallback);
    glfwSetKeyCallback(WINDOW, keyCallback);

    if(glewInit() != GLEW_OK)
    {
        std::cout << "Couldn't initialize glew! \n";
    }

    //Now in GL context

    glViewport(0, 0, SCREENWIDTH.load(), SCREENHEIGHT.load());
    glClearColor(0.5, 0.5, 0.5, 1.0);
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

    jl::Shader gltfShader = getBasicGLTFShader();
    SHADER = &gltfShader;

    jl::ModelAndTextures modelandtexs = jl::ModelLoader::loadModel("assets/models/player.glb");

    while(!glfwWindowShouldClose(WINDOW))
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        if(CAMERA_PROJECTION_UPDATE_REQUESTED.load())
        {
            CAMERA.updateProjection(SCREENWIDTH.load(), SCREENHEIGHT.load(), FOV.load());
        }

        CAMERA.updateWithYawPitch(CAMERA.transform.yaw, CAMERA.transform.pitch);

        glUseProgram(SHADER->shaderID);

        glUniformMatrix4fv(glGetUniformLocation(SHADER->shaderID, "mvp"), 1, GL_FALSE, glm::value_ptr(CAMERA.mvp));
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, modelandtexs.texids.at(0));
        glUniform1i(glGetUniformLocation(SHADER->shaderID, "texture1"), 0);
        glUniform3f(glGetUniformLocation(SHADER->shaderID, "pos"), 0.0, 0.0, 3.0);

        for(jl::ModelGLObjects &mglo : modelandtexs.modelGLObjects)
        {
            glBindVertexArray(mglo.vao);
            //Indent operations on this vertex array object
                glDrawElements(mglo.drawmode, mglo.indexcount, mglo.indextype, nullptr);

            glBindVertexArray(0);
        }

        //Flush out any GL errors
        GLenum err;
        while ((err = glGetError()) != GL_NO_ERROR) {
            std::cerr << "OpenGL error: " << err << std::endl;
        }

        glfwPollEvents();
        glfwSwapBuffers(WINDOW);
    }

    return 0;
}







