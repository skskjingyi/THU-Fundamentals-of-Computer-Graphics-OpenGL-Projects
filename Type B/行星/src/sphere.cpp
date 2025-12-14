#include "sphere.h"
#include <vector>
#include <cmath>

const double PI = 3.14159265358979323846;

void createSphere(
    float radius,
    int stackCount,
    int sectorCount,
    GLuint& sphereVAO,
    GLuint& sphereVBO,
    GLuint& sphereEBO,
    int& indexCount)
{
    std::vector<float> vertices;
    std::vector<unsigned int> indices;

    // -----------generate vertices
    for (int i =0; i <= stackCount; ++i)
    {
        float stackRatio = (float)i / stackCount;
        float stackAngle = PI / 2.0f - stackRatio * PI;

        float y = radius * std::sin(stackAngle);
        float rc = radius * std::cos(stackAngle);

        for (int j = 0; j <= sectorCount; ++j)
        {
            float sectorRatio = (float)j / sectorCount;
            float sectorAngle = sectorRatio * 2.0f * PI;

            float x = rc * std::cos(sectorAngle);
            float z = rc * std::sin(sectorAngle);

            float u = sectorRatio;
            float v = 1.0 - stackRatio;

            // position
            vertices.push_back(x);
            vertices.push_back(y);
            vertices.push_back(z);
            
            // tex coord
            vertices.push_back(u);
            vertices.push_back(v);
        }
    }

    // ========== generate indicies
    // matrix of (stackCount+1) x (sectorCount+1)
    //      k1+j --- k1+j+1
    //        |         |
    //      k2+j --- k2+j+1
    for (int i = 0; i < stackCount; ++i)
    {
        int k1 = i * (sectorCount+1);
        int k2 = (i+1) * (sectorCount+1);

        for (int j = 0; j< sectorCount; ++j)
        {
            if (i != 0)
            {
                indices.push_back(k1+j);
                indices.push_back(k2+j);
                indices.push_back(k1+j+1);
            }
            if(i != (stackCount-1))
            {
                indices.push_back(k1+j+1);
                indices.push_back(k2+j);
                indices.push_back(k2+j+1);
            }
        }
    }

    indexCount = static_cast<int>(indices.size());

    // --------- VAO/VBO/EBO
    glGenVertexArrays(1, &sphereVAO);
    glGenBuffers(1, &sphereVBO);
    glGenBuffers(1, &sphereEBO);

    glBindVertexArray(sphereVAO);

    // VBO: vertex data
    glBindBuffer(GL_ARRAY_BUFFER, sphereVBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size()*sizeof(float), vertices.data(), GL_STATIC_DRAW);

    // EBO: index
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sphereEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size()*sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    // layout (location = 0): vec3 position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // layout (location=1): vec2 position
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 *sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
}



