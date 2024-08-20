//
// Created by wpsimon09 on 20/08/24.
//

#ifndef VERTEXDATA_HPP
#define VERTEXDATA_HPP

namespace VertexData {

    const std::vector<Vertex> cubeVertices = {
        // Front face
        {{-0.5f, -0.5f,  0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f,  1.0f}},  // Vertex 0
        {{ 0.5f, -0.5f,  0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f,  1.0f}},  // Vertex 1
        {{ 0.5f,  0.5f,  0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f,  1.0f}},  // Vertex 2
        {{-0.5f,  0.5f,  0.5f}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f,  1.0f}},  // Vertex 3

        // Back face
        {{-0.5f, -0.5f, -0.5f}, {1.0f, 1.0f, 0.0f}, {0.0f, 0.0f, -1.0f}},  // Vertex 4
        {{ 0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 1.0f}, {0.0f, 0.0f, -1.0f}},  // Vertex 5
        {{ 0.5f,  0.5f, -0.5f}, {0.0f, 1.0f, 1.0f}, {0.0f, 0.0f, -1.0f}},  // Vertex 6
        {{-0.5f,  0.5f, -0.5f}, {0.5f, 0.5f, 0.5f}, {0.0f, 0.0f, -1.0f}},  // Vertex 7

        // Left face
        {{-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 1.0f}, {-1.0f, 0.0f, 0.0f}},  // Vertex 8
        {{-0.5f, -0.5f,  0.5f}, {1.0f, 1.0f, 0.0f}, {-1.0f, 0.0f, 0.0f}},  // Vertex 9
        {{-0.5f,  0.5f,  0.5f}, {0.0f, 1.0f, 1.0f}, {-1.0f, 0.0f, 0.0f}},  // Vertex 10
        {{-0.5f,  0.5f, -0.5f}, {0.5f, 0.5f, 0.5f}, {-1.0f, 0.0f, 0.0f}},  // Vertex 11

        // Right face
        {{ 0.5f, -0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, { 1.0f, 0.0f, 0.0f}},  // Vertex 12
        {{ 0.5f, -0.5f,  0.5f}, {1.0f, 0.0f, 1.0f}, { 1.0f, 0.0f, 0.0f}},  // Vertex 13
        {{ 0.5f,  0.5f,  0.5f}, {0.0f, 0.0f, 1.0f}, { 1.0f, 0.0f, 0.0f}},  // Vertex 14
        {{ 0.5f,  0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}, { 1.0f, 0.0f, 0.0f}},  // Vertex 15

        // Top face
        {{-0.5f,  0.5f, -0.5f}, {1.0f, 0.0f, 1.0f}, {0.0f, 1.0f, 0.0f}},   // Vertex 16
        {{-0.5f,  0.5f,  0.5f}, {1.0f, 1.0f, 0.0f}, {0.0f, 1.0f, 0.0f}},   // Vertex 17
        {{ 0.5f,  0.5f,  0.5f}, {0.0f, 1.0f, 1.0f}, {0.0f, 1.0f, 0.0f}},   // Vertex 18
        {{ 0.5f,  0.5f, -0.5f}, {0.5f, 0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},   // Vertex 19

        // Bottom face
        {{-0.5f, -0.5f, -0.5f}, {1.0f, 1.0f, 0.0f}, {0.0f, -1.0f, 0.0f}},  // Vertex 20
        {{-0.5f, -0.5f,  0.5f}, {1.0f, 0.0f, 1.0f}, {0.0f, -1.0f, 0.0f}},  // Vertex 21
        {{ 0.5f, -0.5f,  0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, -1.0f, 0.0f}},  // Vertex 22
        {{ 0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, -1.0f, 0.0f}}   // Vertex 23
    };

    const std::vector<uint32_t> cubeIndices = {
        // Front face
        0, 1, 2, 2, 3, 0,

        // Right face
        13, 12, 15, 15, 14, 13,

        // Back face
        6, 5, 4, 4, 7, 6,

        // Left face
        8, 9, 10, 10, 11, 8,

        // Top face
        16, 17, 18, 18, 19, 16,

        // Bottom face
        21, 20, 23, 23, 22, 21
    };

    const std::vector<Vertex> planeVertices = {
        {{-0.5f, 0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}},  // Bottom-left
        {{ 0.5f, 0.0f, -0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f, 0.0f}},  // Bottom-right
        {{ 0.5f, 0.0f,  0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f, 0.0f}},  // Top-right
        {{-0.5f, 0.0f,  0.5f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f, 0.0f}}   // Top-left
    };
    
    const std::vector<uint32_t> planeIndices = {
        0, 1, 2, 2, 3, 0
    };
}

#endif //VERTEXDATA_HPP
