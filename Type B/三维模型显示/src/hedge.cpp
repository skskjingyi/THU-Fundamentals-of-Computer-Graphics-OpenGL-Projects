#include "hedge.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <unordered_map>
#include <glm/glm.hpp>
#include <vector>


struct FaceIndex { int v0; int v1; int v2; };

struct EdgeKey {
    int from;
    int to;

    bool operator==(const EdgeKey& other) const {
        return from == other.from && to == other.to;
    }
};

struct EdgeKeyHash {
    std::size_t operator()(const EdgeKey& k) const {
        // simple hash combine
        return std::hash<int>()(k.from) ^ (std::hash<int>()(k.to) << 1);
    }
};

Hedge::~Hedge()
{
    clear();
}

void Hedge::clear()
{
    for (auto v : vertices) delete v;
    for (auto f : faces) delete f;
    for (auto e : edges) delete e;

    vertices.clear();
    faces.clear();
    edges.clear();
}

bool Hedge::loadFromOBJ(const std::string& path)
{
    clear();
    std::ifstream in(path);
    std::string line;

    std::vector<glm::vec3> tmpPositions;
    std::vector<FaceIndex> tmpFaces;

    while (std::getline(in, line))
    {
        std::stringstream ss(line);
        std::string tag;
        ss >> tag;

        if (tag == "v")
        {
            float x,y,z;
            ss >> x >> y >> z;
            tmpPositions.push_back(glm::vec3(x,y,z));
        }
        else if (tag == "f")
        {
            std::string vStr;
            int idx[3];
            int count = 0;

            // read up to 3 vertices for this face
            while (count < 3 && (ss >> vStr)) {
                // handle formats like "3/2/1" or "3//1"
                size_t slashPos = vStr.find('/');
                if (slashPos != std::string::npos) {
                    vStr = vStr.substr(0, slashPos); // keep only the vertex index before '/'
                }

                int vi = std::stoi(vStr) - 1; // convert 1-based OBJ index to 0-based

                idx[count] = vi;
                count++;
            }

            if (count == 3) {
                FaceIndex f;
                f.v0 = idx[0];
                f.v1 = idx[1];
                f.v2 = idx[2];
                tmpFaces.push_back(f);
            }
        }

    }

    in.close();
    // debug check: make sure all face indices are valid
    for (const auto& f : tmpFaces) {
        if (f.v0 < 0 || f.v1 < 0 || f.v2 < 0 ||
            f.v0 >= (int)tmpPositions.size() ||
            f.v1 >= (int)tmpPositions.size() ||
            f.v2 >= (int)tmpPositions.size()) 
        {
            std::cout << "Invalid face indices: "
                    << f.v0 << ", " << f.v1 << ", " << f.v2
                    << " with vertex count = " << tmpPositions.size()
                    << std::endl;
            return false; // or exit(1) while debugging
        }
    }
    // create HEvertex for each position

    vertices.reserve(tmpPositions.size());
    for (size_t i =0; i < tmpPositions.size(); i++)
    {
        HEVertex* v = new HEVertex();
        v->position = tmpPositions[i];
        v->index = static_cast<int>(i);
        vertices.push_back(v);
    }

    // build faces and half edges
    std::unordered_map<EdgeKey, HalfEdge*, EdgeKeyHash> edgeMap;
    edgeMap.reserve(tmpFaces.size() * 3);
    faces.reserve(tmpFaces.size());
    edges.reserve(tmpFaces.size()*3);

    for (const auto& fIdx : tmpFaces) {
        HEFace* face = new HEFace();
        faces.push_back(face);

        int a = fIdx.v0;
        int b = fIdx.v1;
        int c = fIdx.v2;

        // Create 3 half-edges: a->b, b->c, c->a
        HalfEdge* e0 = new HalfEdge();
        HalfEdge* e1 = new HalfEdge();
        HalfEdge* e2 = new HalfEdge();

        // Set from/to indices
        e0->fromIndex = a; e0->toIndex = b;
        e1->fromIndex = b; e1->toIndex = c;
        e2->fromIndex = c; e2->toIndex = a;

        // Edges point TO their 'toIndex' vertex
        e0->vert = vertices[b];
        e1->vert = vertices[c];
        e2->vert = vertices[a];

        // Face pointers
        e0->face = face;
        e1->face = face;
        e2->face = face;
        face->edge = e0;
        
        // Next/prev links (counter-clockwise)
        e0->next = e1; e1->next = e2; e2->next = e0;
        e0->prev = e2; e1->prev = e0; e2->prev = e1;

        // Give each vertex an outgoing edge if it doesn't have one yet
        if (!vertices[a]->edge) vertices[a]->edge = e0;
        if (!vertices[b]->edge) vertices[b]->edge = e1;
        if (!vertices[c]->edge) vertices[c]->edge = e2;

         // Add edges to list
        edges.push_back(e0);
        edges.push_back(e1);
        edges.push_back(e2);

        auto addEdgeToMap = [&](HalfEdge* e) {
            EdgeKey key { e->fromIndex, e->toIndex };
            EdgeKey twinKey { e->toIndex, e->fromIndex };
            
            auto it = edgeMap.find(twinKey);
            if (it != edgeMap.end()) {
                // Found opposite edge -> set twins
                e->twin = it->second;
                it->second->twin = e;
            }

            edgeMap[key] = e;
        };

        addEdgeToMap(e0);
        addEdgeToMap(e1);
        addEdgeToMap(e2);
    }

    return true;
}

// build array for OpenGL
void Hedge::buildVertexArray(std::vector<glm::vec3>& outPositions) const
{
    outPositions.clear();
    outPositions.reserve(vertices.size());

    for (auto v:vertices)
    {
        outPositions.push_back(v->position);
    }
}

void Hedge::buildFaceIndexArray(std::vector<unsigned int>& outIndices) const
{
    outIndices.clear();
    outIndices.reserve(faces.size() * 3);

    for (auto face : faces) {
        if (!face || !face->edge) continue;

        HalfEdge* e0 = face->edge;
        HalfEdge* e1 = e0->next;
        HalfEdge* e2 = e1->next;

        // Use fromIndex (0-based) for each corner
        outIndices.push_back(static_cast<unsigned int>(e0->fromIndex));
        outIndices.push_back(static_cast<unsigned int>(e1->fromIndex));
        outIndices.push_back(static_cast<unsigned int>(e2->fromIndex));
    }
}

void Hedge::buildEdgeIndexArray(std::vector<unsigned int>& outIndices) const
{
    outIndices.clear();
    outIndices.reserve(edges.size() * 2);

    for (auto e : edges) {
        if (!e) continue;

        // Avoid adding each undirected edge twice.
        // Only emit if:
        //  - either no twin, or
        //  - this pointer is "less" than twin (arbitrary but consistent)
        if (e->twin && e > e->twin) {
            continue;
        }

        outIndices.push_back(static_cast<unsigned int>(e->fromIndex));
        outIndices.push_back(static_cast<unsigned int>(e->toIndex));
    }
}
