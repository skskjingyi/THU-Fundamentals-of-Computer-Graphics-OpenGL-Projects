#ifndef PLANET_H
#define PLANET_H
#pragma once
#include <glm/glm.hpp>
#include <vector>
#include <string>
#include <GLFW/glfw3.h>
#include <glad/glad.h>

struct OrbitalElements {
    double N1, N2;
    double i1, i2;
    double w1, w2;
    double a1, a2;
    double e1, e2;
    double M1, M2;
    double rotPeriod;
    int centerOfOrbit;
};

struct Planet {
    OrbitalElements elem;

    glm::dvec3 posAU;
    double selfAngle;

    float radiusGL;
    GLuint texture;
    GLuint nameTexture;
};

class PlanetSystem {
public:
    PlanetSystem();
    ~PlanetSystem() {}

    void init();                       // load planet data, load textures
    void update(double deltaDays);     // compute orbital positions
    void draw(const glm::mat4& view, 
              const glm::mat4& proj,
              unsigned int sphereVAO,
              int sphereIndexCount,
              GLuint shaderProgram);   // render planets

private:
    glm::dvec3 computeOrbitPositionAU(const Planet& p, double timeD);
    void loadOrbitalElements();
    void loadTextures();
    void createName( GLuint& nameVAO, GLuint& nameVBO, GLuint& nameEBO);
    std::vector<Planet> planets;
    double currTimeDays;
};
#endif