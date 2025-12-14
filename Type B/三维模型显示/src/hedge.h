#ifndef HEDGE_H
#define HEDGE_H
#include <glm/glm.hpp>
#include <vector>
#include <string>

// Forward declarations
struct HEVertex;
struct HEFace;
struct HalfEdge;

struct HEVertex {
    glm::vec3 position;   // 3D position
    HalfEdge* edge = nullptr; // one outgoing half-edge
    int index = -1;        // index in the vertex array (for OpenGL)
};

struct HEFace
{
    HalfEdge* edge = nullptr;  // one halfe edge on this face
};

struct HalfEdge
{
    HEVertex* vert = nullptr;  // vertex this edge points TO
    HEFace*   face = nullptr;  // face this edge belongs to
    HalfEdge* next = nullptr;  // next edge around the face
    HalfEdge* prev = nullptr;  // previous edge around the face
    HalfEdge* twin = nullptr;  // opposite (neighbor) edge

    // For convenience when building / exporting
    int fromIndex = -1;   // starting vertex index (0-based)
    int toIndex   = -1;   // ending vertex index (0-based)
};

class Hedge
{   
public:
    std::vector<HEVertex*> vertices;
    std::vector<HEFace*>   faces;
    std::vector<HalfEdge*> edges;

    Hedge() = default;
    ~Hedge();

    // Delete all data
    void clear();

    // Load from a simple OBJ file (only v and f, triangles)
    bool loadFromOBJ(const std::string& path);

    // Build arrays for OpenGL:

    // Positions for VBO: size = numVertices
    void buildVertexArray(std::vector<glm::vec3>& outPositions) const;

    // Triangle indices (faces): size = numFaces * 3
    void buildFaceIndexArray(std::vector<unsigned int>& outIndices) const;

    // Edge indices (for wireframe): 2 indices per edge (each edge only once)
    void buildEdgeIndexArray(std::vector<unsigned int>& outIndices) const;
};


#endif