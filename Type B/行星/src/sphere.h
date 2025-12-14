#ifndef SPHERE_H
#define SPHERE_H

#include <glad/glad.h>

void createSphere(
    float radius,
    int stackCount,
    int sectorCount,
    GLuint& sphereVAO,
    GLuint& sphereVBO,
    GLuint& sphereEBO,
    int& indexCount
);

#endif

