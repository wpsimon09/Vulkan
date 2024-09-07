//
// Created by wpsimon09 on 18/08/24.
//

#include "Camera.hpp"

#include <future>

Camera::Camera(GLFWwindow* window, glm::vec3 center, glm::vec3 up, float radius, float minRadius, float azimuthAngle, float polarAngle) {
    this->center = center;
    this->worldUp = up;

    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    this->projection = glm::perspective(glm::radians(65.0f), (float)width / (float)height, 0.1f, 700.0f);
    this->farPlane = 700.0f;;
    this->nearPlane = 0.1f;

    this->radius = radius;
    this->minRadius = minRadius;
    this->azimuthAngle = azimuthAngle;
    this->polarAngle = polarAngle;
    this->position = getEye();
}

void Camera::rotateAzimutn(float radians) {
    azimuthAngle += radians;
    const auto fullCircle = 2.0f * glm::pi<float>();
    azimuthAngle = fmodf(azimuthAngle, fullCircle);
    if(azimuthAngle<0.0f){
        azimuthAngle = azimuthAngle+fullCircle;
    }
    this->position = getEye();
}

void Camera::rotatePolar(float radains) {
    polarAngle += radains;

    const auto polarCap = glm::pi<float>() /2 - 0.001f;

    if (this->polarAngle > polarCap) {
        this->polarAngle = polarCap;
    }

    if (this->polarAngle < -polarCap) {
        this->polarAngle = -polarCap;
    }
    this->position = getEye();
}

void Camera::zoom(float by) {
    this->radius -= by;

    if(radius< minRadius){
        radius = minRadius;
    }
    this->position = getEye();
}

void Camera::moveHorizontal(float distance) {
    const auto pos = getEye();
    const glm::vec3 viewVector = glm::normalize(this->position - this->center);
    const glm::vec3 strafeVector = glm::normalize(glm::cross(viewVector, worldUp));
    center += worldUp + distance;
    this->position = getEye();
}

void Camera::moveVertical(float distance) {
    center+= worldUp + distance;
    this->position = getEye();
}

void Camera::processResize(int newWidht, int newHeight) {
    ;
    this->projection = glm::perspective(glm::radians(65.0f), (float)newWidht / (float)newHeight, 0.1f, 470.0f);
    this->farPlane = getFarPlane();
    this->nearPlane = 0.1f;
    this->position = getEye();
}

void Camera::update() {

}

glm::vec3 Camera::getEye() {
    const auto sineAzimuth = sin(azimuthAngle);
    const auto cosineAzimuzh = cos(azimuthAngle);
    const auto sinePolar = sin(polarAngle);
    const auto cosinePolar = cos(polarAngle);

    const auto x = center.x + radius * cosinePolar * cosineAzimuzh;
    const auto y = center.y + radius * sinePolar;
    const auto z = center.z + radius * cosinePolar * sineAzimuth;

    return {x,y,z};
}



