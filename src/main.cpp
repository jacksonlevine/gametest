
#include <iostream>
#include <atomic>
#include <string>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

// Define these only in *one* .cc file.
#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
// #define TINYGLTF_NOEXCEPTION // optional. disable exception handling.
#include "tiny_gltf.h"

GLFWwindow* window = nullptr;

std::atomic<int> screenwidth = 1280;
std::atomic<int> screenheight = 720;

void frameBufferCallback(GLFWwindow* window, int width, int height)
{
    std::cout << std::to_string(width) << " " << std::to_string(height) << "\n";
    glViewport(0, 0, width, height);
}

int main() {

    if (glfwInit() != GLFW_TRUE)
    {
        std::cout << "Couldn't initialize glfw.\n";
    }

    window = glfwCreateWindow(screenwidth.load(), screenheight.load(), "Game", nullptr, nullptr);
    glfwMakeContextCurrent(window);

    glfwSetFramebufferSizeCallback(window, frameBufferCallback);

    if(glewInit() != GLEW_OK)
    {
        std::cout << "Couldn't initialize glew! \n";
    }

    glClearColor(0.5, 0.5, 0.5, 1.0);

    while(!glfwWindowShouldClose(window))
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glfwPollEvents();
        glfwSwapBuffers(window);
    }

    return 0;
}

