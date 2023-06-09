/*
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/normal.hpp>
#include <vector>

#include "Common.hpp"

// For develeppement purpose
#include <iostream>

#include "Surface.hpp"

template <size_t M, size_t N>
BezierSurface<M, N>::BezierSurface(const std::array<glm::vec3, M * N>&& ctrlGrid, size_t edges)
    : m_ctrlGrid(ctrlGrid), m_meshEdges(edges) {
    m_mesh = {std::vector<Vertex>(), std::vector<unsigned int>()};
    evaluate();
}

template <size_t M, size_t N>
const Mesh& BezierSurface<M, N>::mesh() const {
    return m_mesh;
}

template <size_t M, size_t N>
void BezierSurface<M, N>::setMeshEdges(size_t edges) {
    m_meshEdges = edges;
    evaluate();
}

template <size_t M, size_t N>
void BezierSurface<M, N>::evaluate() {
    std::vector<glm::vec3> q_points(M * m_meshEdges, glm::vec3(0.0, 0.0, 0.0));
    for (int i = 0; i < M; ++i) {
        for (int j = 0; j < m_meshEdges; ++j) {
            float v = float(j) / float(m_meshEdges - 1);
            q_points[j * M + i] = deCasteljau(v, m_ctrlGrid.cbegin() + i * N, m_ctrlGrid.cbegin() + i * N + N);
        }
    }

    for (int i = 0; i < q_points.size() / M; ++i) {
        for (int j = 0; j < m_meshEdges; ++j) {
            float u = float(j) / float(m_meshEdges - 1);

            Vertex vertex;
            vertex.position = deCasteljau(u, q_points.cbegin() + i * M, q_points.cbegin() + (i + 1) * M);
            vertex.normal = glm::vec3(0.0, 0.0, 0.0);
            vertex.tex_coords = glm::vec2(1.0, 1.0);

            m_mesh.vertices.push_back(vertex);
        }
    }

    calculateIndices();
    // TODO: calculate normals with bezier derivative
    calculateNormals();
}

template <size_t M, size_t N>
void BezierSurface<M, N>::calculateIndices() {
    m_mesh.indices.reserve(m_meshEdges * m_meshEdges * 6);

    for (unsigned int i = 0; i < m_meshEdges - 1; ++i) {
        for (unsigned int j = 0; j < m_meshEdges - 1; ++j) {
            // first triangle
            m_mesh.indices.push_back(i * m_meshEdges + j);
            m_mesh.indices.push_back(i * m_meshEdges + j + 1);
            m_mesh.indices.push_back(i * m_meshEdges + j + m_meshEdges);
            // second triangle
            m_mesh.indices.push_back(i * m_meshEdges + j + 1);
            m_mesh.indices.push_back(i * m_meshEdges + j + m_meshEdges);
            m_mesh.indices.push_back(i * m_meshEdges + j + m_meshEdges + 1);
        }
    }
}

template <size_t M, size_t N>
void BezierSurface<M, N>::calculateNormals() {
    for (int i = 0; i < m_meshEdges - 1; i++) {
        for (int j = 0; j < m_meshEdges - 1; j++) {
            auto p0 = &m_mesh.vertices[i * m_meshEdges + j].position;
            auto p1 = &m_mesh.vertices[i * m_meshEdges + j + 1].position;
            auto p2 = &m_mesh.vertices[i * m_meshEdges + j + m_meshEdges].position;
            auto p3 = &m_mesh.vertices[i * m_meshEdges + j + m_meshEdges + 1].position;

            auto v0 = *p2 - *p0;
            auto v1 = *p1 - *p0;
            auto v2 = *p3 - *p1;

            if (i == m_meshEdges - 2) {
                auto v3 = *p3 - *p2;
                m_mesh.vertices[(i + 1) * m_meshEdges + j].normal += glm::normalize(glm::cross(v0, v3));

                if (i == j) {
                    m_mesh.vertices[(i + 1) * m_meshEdges + j + 1].normal += glm::normalize(glm::cross(v2, v3));
                }
            }

            if (j == m_meshEdges - 2) {
                m_mesh.vertices[i * m_meshEdges + j + 1].normal += glm::normalize(glm::cross(v2, v1));
            }

            m_mesh.vertices[i * m_meshEdges + j].normal = glm::normalize(glm::cross(v0, v1));
        }
    }
}