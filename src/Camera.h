//
// Created by jack on 10/13/2024.
//

#ifndef CAMERA_H
#define CAMERA_H
#include "Transform.h"
#include <iostream>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>

namespace jl {

class Camera {
public:
    jl::Transform transform;
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 projection;
    glm::mat4 mvp;
    Camera()
        : model(glm::mat4(1.0f)), // Identity matrix
          view(glm::mat4(1.0f)),
          projection(glm::mat4(1.0f)),
          mvp(glm::mat4(1.0f))
        {}
    void updateProjection(int screenwidth, int screenheight, float fov);
    void updateWithYawPitch(float nyaw, float npitch);
    void printCameraValues() const {
        std::cout << "Camera Values:\n";

        // Print Transform values
        std::cout << "Transform:\n";
        std::cout << "  Position: " << glm::to_string(transform.position) << "\n";
        std::cout << "  Rotation (Yaw, Pitch): (" << transform.yaw << ", " << transform.pitch << ")\n";

        // Print Matrix values
        std::cout << "Model Matrix:\n" << glm::to_string(model) << "\n";
        std::cout << "View Matrix:\n" << glm::to_string(view) << "\n";
        std::cout << "Projection Matrix:\n" << glm::to_string(projection) << "\n";
        std::cout << "MVP Matrix:\n" << glm::to_string(mvp) << "\n";
    }
};

} // jl

#endif //CAMERA_H
