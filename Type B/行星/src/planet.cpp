#include "planet.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <cmath>

extern GLuint loadTexture(const char* path);

#define DEG2RAD 0.017453292519943295
static const double AU_TO_GL = 5.0;     // 1 AU = 5 OpenGL units
const double PI = 3.14159265358979323846;
GLuint nameVBO, nameVAO, nameEBO;

PlanetSystem::PlanetSystem() {
    currTimeDays = 0.0;
}

void PlanetSystem::init() {
    loadOrbitalElements();
    loadTextures();
    createName(nameVAO, nameVBO, nameEBO);
}

void PlanetSystem::loadOrbitalElements() {
    planets.resize(6);  // Sun + 5 planets

    // -------- SUN --------
    planets[0].elem = {0,0,0,0,0,0,0,0,0,0,0,0,25.05,0};
    planets[0].radiusGL = 1.2f;

    // -------- MERCURY --------
    planets[1].elem = {48.3313,0.0000324587, 7.0047,0.0000000500,
                       29.1241,0.0000101444, 0.387098,0,
                       0.205635,0.000000000559,
                       168.6562,4.0923344368, 58.646, 0};
    planets[1].radiusGL = 0.3f;

    // -------- VENUS --------
    planets[2].elem = {76.6799,0.0000246590, 3.3946,0.0000000275,
                       54.8910,0.0000138374, 0.723330,0,
                       0.006773,-0.000000001302,
                       48.0052,1.6021302244, 243.0185, 0};
    planets[2].radiusGL = 0.4f;

    // -------- EARTH --------
    planets[3].elem = {174.873,0, 0.00005,0,
                       102.94719,0, 1.0,0,
                       0.01671022,0,
                       357.529,0.985608, 0.997, 0};
    planets[3].radiusGL = 0.45f;

    // -------- MARS --------
    planets[4].elem = {49.5574,0.0000211081, 1.8497,-0.0000000178,
                       286.5016,0.0000292961, 1.523688,0,
                       0.093405,0.000000002516,
                       18.6021,0.5240207766, 1.025957, 0};
    planets[4].radiusGL = 0.35f;

    // -------- JUPITER --------
    planets[5].elem = {100.4542,0.0000276854, 1.3030,-0.0000001557,
                       273.8777,0.0000164505, 5.20256,0,
                       0.048498,0.000000004469,
                       19.8950,0.0830853001, 0.4135, 0};
    planets[5].radiusGL = 0.7f;
}

void PlanetSystem::loadTextures() {
    planets[0].texture = loadTexture("resources/tex/sun.jpg");
    planets[1].texture = loadTexture("resources/tex/mercury.jpg");
    planets[2].texture = loadTexture("resources/tex/venus.jpg");
    planets[3].texture = loadTexture("resources/tex/earth.jpg");
    planets[4].texture = loadTexture("resources/tex/mars.jpg");
    planets[5].texture = loadTexture("resources/tex/moon.jpg");

    // name texure
    planets[0].nameTexture = loadTexture("resources/tex/sunTag.png");
    planets[1].nameTexture = loadTexture("resources/tex/mercuryTag.png");
    planets[2].nameTexture = loadTexture("resources/tex/venusTag.png");
    planets[3].nameTexture = loadTexture("resources/tex/earthTag.png");
    planets[4].nameTexture = loadTexture("resources/tex/marsTag.png");
    planets[5].nameTexture = loadTexture("resources/tex/moonTag.png");
}

glm::dvec3 PlanetSystem::computeOrbitPositionAU(const Planet& p, double t) {
    const auto& e = p.elem;

    double N = (e.N1 + e.N2 * t) * DEG2RAD;
    double inc = (e.i1 + e.i2 * t) * DEG2RAD;
    double w = (e.w1 + e.w2 * t) * DEG2RAD;
    double a =  e.a1 + e.a2 * t;
    double ecc = e.e1 + e.e2 * t;
    double M = (e.M1 + e.M2 * t) * DEG2RAD;

    double E = M + ecc * std::sin(M) * (1.0 + ecc * std::cos(M));

    double xv = a * (std::cos(E) - ecc);
    double yv = a * (std::sqrt(1.0 - ecc * ecc) * std::sin(E));
    double v = std::atan2(yv, xv);
    double r = std::sqrt(xv * xv + yv * yv);

    double cosN = std::cos(N);
    double sinN = std::sin(N);
    double cosi = std::cos(inc);
    double sini = std::sin(inc);
    double cosvw = std::cos(v + w);
    double sinvw = std::sin(v + w);

    double xh = r * (cosN * cosvw - sinN * sinvw * cosi);
    double zh = -r * (sinN * cosvw + cosN * sinvw * cosi);
    double yh = r * (sinvw * sini);

    return glm::dvec3(xh, yh, zh);
}

void PlanetSystem::update(double deltaDays) {
    currTimeDays += deltaDays;

    for (auto& p : planets) {
        p.posAU = computeOrbitPositionAU(p, currTimeDays);

        if (p.elem.rotPeriod > 0)
            p.selfAngle += (deltaDays / p.elem.rotPeriod) * 2.0 * PI;
    }
}

void PlanetSystem::createName(GLuint& nameVAO, GLuint& nameVBO, GLuint& nameEBO)
{
    float nameTag[] = {
    -0.5f,  0.25f, 0.0f,   0.0f, 1.0f,  // 0: left-top
     0.5f,  0.25f, 0.0f,  1.0f, 1.0f,  // 1: right-top
     0.5f, -0.25f, 0.0f,  1.0f, 0.0f,  // 2: right-bottom
    -0.5f, -0.25f, 0.0f,  0.0f, 0.0f   // 3: left-bottom
    };

    unsigned int nameIndicies[] = {
        0, 1, 2,
        2, 3, 0
    };

     // --------- VAO/VBO/EBO
    glGenVertexArrays(1, &nameVAO);
    glGenBuffers(1, &nameVBO);
    glGenBuffers(1, &nameEBO);

    glBindVertexArray(nameVAO);

    // VBO: vertex data
    glBindBuffer(GL_ARRAY_BUFFER, nameVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(nameTag), nameTag, GL_STATIC_DRAW);

    // EBO: index
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, nameEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(nameIndicies), nameIndicies, GL_STATIC_DRAW);

    // layout (location = 0): vec3 position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // layout (location=1): vec2 position
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 *sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
}


void PlanetSystem::draw(const glm::mat4& view,
                        const glm::mat4& proj,
                        GLuint sphereVAO,
                        int indexCount,
                        GLuint shaderProgram)
{
    glUseProgram(shaderProgram);

    GLint modelLoc = glGetUniformLocation(shaderProgram, "model");
    GLint viewLoc  = glGetUniformLocation(shaderProgram, "view");
    GLint projLoc  = glGetUniformLocation(shaderProgram, "projection");

    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(proj));

    for (auto& p : planets) {
        glm::vec3 posGL = glm::vec3(
            p.posAU.x * AU_TO_GL,
            p.posAU.y * AU_TO_GL,
            p.posAU.z * AU_TO_GL
        );

        // draw sphere
        glBindVertexArray(sphereVAO);
        glm::mat4 model(1.0f);
        model = glm::translate(model, posGL);
        model = glm::rotate(model, (float)p.selfAngle, glm::vec3(0,1,0));
        model = glm::scale(model, glm::vec3(p.radiusGL));

        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

        glBindTexture(GL_TEXTURE_2D, p.texture);
        glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);

        // draw name
        glm::vec3 namePos = posGL + glm::vec3(0.0f, p.radiusGL * -1.5f, 0.0f);
        glm::mat4 nameModel(1.0f);
        nameModel = glm::translate(nameModel, namePos);
        nameModel = glm::scale(nameModel, glm::vec3(1.0f)); // adjust size
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(nameModel));

        glBindVertexArray(nameVAO);
        glBindTexture(GL_TEXTURE_2D, p.nameTexture);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    }
}
