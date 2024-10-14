//
// Created by jack on 10/13/2024.
//

#include "Camera.h"

namespace jl {
    void Camera::updateProjection(int screenwidth, int screenheight, float fov)
    {
        std::cout << "Updating projection with: \n" << " - " << std::to_string(screenwidth) << "\n - " << std::to_string(screenheight);
        projection = glm::perspective(
            glm::radians(fov),
            static_cast<float>(screenwidth) / static_cast<float>(screenheight),
            0.1f,
            250.0f
            );
        mvp = projection * view * model;
    }

    void Camera::updateWithYawPitch(float nyaw, float npitch)
    {
        std::cout << "Updating with yaw pitch \n";
        transform.updateWithYawPitch(nyaw, npitch);
        view = glm::lookAt(transform.position,
            transform.position + transform.direction,
            transform.up);
        mvp = projection * view * model;
    }
} // jl