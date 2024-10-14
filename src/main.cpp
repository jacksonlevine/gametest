
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

jl::Camera CAMERA;
jl::Shader* SHADER; //This must be initialized after the GL context, so it's a pointer

bool FIRSTMOUSE = true;

void frameBufferCallback(GLFWwindow* window, int width, int height)
{
    std::cout << std::to_string(width) << " " << std::to_string(height) << "\n";
    SCREENWIDTH.store(width); SCREENHEIGHT.store(height);
    glViewport(0, 0, width, height);
}

void cursorPosCallback(GLFWwindow* window, double xpos, double ypos)
{
    static double lastx = 0.0;
    static double lasty = 0.0;

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

void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
    if(action)
    {
        if(button == GLFW_MOUSE_BUTTON_1)
        {
            glfwSetInputMode(WINDOW, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        } else
        {
            glfwSetInputMode(WINDOW, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
    }
}

void shittydraw()
{
    // Vertex data for a simple triangle
    float positions[] = {
        -0.5f, -0.5f, 0.0f,
         0.5f, -0.5f, 0.0f,
         0.0f,  0.5f, 0.0f
    };

    float texCoords[] = {
        0.0f, 0.0f,
        1.0f, 0.0f,
        0.5f, 1.0f
    };

    // Create and bind a VAO
        static GLuint VAO = 0;
        static GLuint posVBO;
        static GLuint texVBO;

    if (VAO == 0)
    {
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &texVBO);
        glGenBuffers(1, &posVBO);
    }

    glBindVertexArray(VAO);

    // Create and bind VBO for positions


    glBindBuffer(GL_ARRAY_BUFFER, posVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(positions), positions, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Create and bind VBO for texture coordinates


    glBindBuffer(GL_ARRAY_BUFFER, texVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(texCoords), texCoords, GL_STATIC_DRAW);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);

    // Use your shader program
    glUseProgram(SHADER->shaderID);

    // Set up a simple orthographic projection
    glm::mat4 projection = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f);
    glUniformMatrix4fv(glGetUniformLocation(SHADER->shaderID, "mvp"), 1, GL_FALSE, glm::value_ptr(projection));

    // Set the position uniform to (0, 0, 0)
    glUniform3f(glGetUniformLocation(SHADER->shaderID, "pos"), 0.0f, 0.0f, 0.0f);

    // Create a simple white texture
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    unsigned char white[] = {255, 255, 255, 255};
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, white);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // Bind the texture to texture unit 0
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    glUniform1i(glGetUniformLocation(SHADER->shaderID, "texture1"), 0);

    // Draw the triangle
    glDrawArrays(GL_TRIANGLES, 0, 3);



}

int main() {

    static int LAST_SCREEN_HEIGHT = 0;
    static int LAST_SCREEN_WIDTH = 0;
    static float LAST_FOV = 0.0f;

    if (glfwInit() != GLFW_TRUE)
    {
        std::cout << "Couldn't initialize glfw.\n";
    }

    WINDOW = glfwCreateWindow(SCREENWIDTH.load(), SCREENHEIGHT.load(), "Game", nullptr, nullptr);
    glfwMakeContextCurrent(WINDOW);

    glfwSetFramebufferSizeCallback(WINDOW, frameBufferCallback);
    glfwSetCursorPosCallback(WINDOW, cursorPosCallback);
    glfwSetMouseButtonCallback(WINDOW, mouseButtonCallback);

    if(glewInit() != GLEW_OK)
    {
        std::cout << "Couldn't initialize glew! \n";
    }

    jl::Shader gltfShader = getBasicGLTFShader();
    SHADER = &gltfShader;

    jl::ModelAndTextures modelandtexs = jl::ModelLoader::loadModel("assets/models/AntiqueCamera.glb");




    glViewport(0, 0, SCREENWIDTH.load(), SCREENHEIGHT.load());
    glClearColor(0.5, 0.5, 0.5, 1.0);
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);






    while(!glfwWindowShouldClose(WINDOW))
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        //shittydraw();

        const int currentSW = SCREENWIDTH.load();
        const int currentSH = SCREENHEIGHT.load();
        const float currentFOV = FOV.load();

        if(LAST_FOV != currentFOV || LAST_SCREEN_WIDTH != currentSW || LAST_SCREEN_HEIGHT != currentSH)
        {
            CAMERA.updateProjection(currentSW, currentSH, currentFOV);
            LAST_FOV = currentFOV;
            LAST_SCREEN_WIDTH = currentSW;
            LAST_SCREEN_HEIGHT = currentSH;
        }

        CAMERA.updateWithYawPitch(CAMERA.transform.yaw, CAMERA.transform.pitch);
        //CAMERA.printCameraValues();
        glUseProgram(SHADER->shaderID);


        //glUniformMatrix4fv(glGetUniformLocation(SHADER->shaderID, "mvp"), 1, GL_FALSE, glm::value_ptr(CAMERA.mvp));


        for(jl::ModelGLObjects &mglo : modelandtexs.modelGLObjects)
        {
            //std::cout << "Index count: " << std::to_string(mglo.indexcount) << "\n";
            glBindVertexArray(mglo.vao);

            glUniformMatrix4fv(glGetUniformLocation(SHADER->shaderID, "mvp"), 1, GL_FALSE, glm::value_ptr(CAMERA.mvp));


            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, modelandtexs.texids.at(0));
            glUniform1i(glGetUniformLocation(SHADER->shaderID, "texture1"), 0);


            glUniform3f(glGetUniformLocation(SHADER->shaderID, "pos"), 0.0, 0.0, 7.0);
            glDrawElements(mglo.drawmode, mglo.indexcount, mglo.indextype, nullptr);
    
            glBindVertexArray(0);
        }
        GLenum err;
        while ((err = glGetError()) != GL_NO_ERROR) {
            std::cerr << "OpenGL error: " << err << std::endl;
        }



        glfwPollEvents();
        glfwSwapBuffers(WINDOW);
    }

    return 0;
}







