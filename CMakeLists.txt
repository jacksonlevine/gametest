cmake_minimum_required(VERSION 3.28)

# Set the vcpkg toolchain file, make sure you do this before project()
if(DEFINED ENV{VCPKG_ROOT})
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
else()
    message(FATAL_ERROR "VCPKG_ROOT environment variable not set")
endif()

project(MyGame)

set(CMAKE_CXX_STANDARD 23)

find_package(glfw3 CONFIG REQUIRED)
find_package(GLEW REQUIRED)
find_package(glm CONFIG REQUIRED)

add_executable(MyGame src/main.cpp
        src/ModelLoader.cpp
        src/Shader.cpp
        src/Camera.cpp
        src/Transform.cpp
        src/Matrix.cpp
        src/Trackball.cpp
        src/Skinning.cpp
)

target_link_libraries(MyGame PRIVATE glfw glm::glm GLEW::GLEW)
