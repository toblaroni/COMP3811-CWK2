#include "cube.hpp"

#include <numbers>

SimpleMeshData make_cube(Material aMaterial, Mat44f aPreTransform) 
{
    // Calculate normal matrix
    Mat33f const N = mat44_to_mat33(transpose(invert(aPreTransform)));

    // Define vertices of a unit cube centered at origin
    Vec3f vertices[] = {
        {-0.5f, -0.5f, -0.5f}, // 0
        { 0.5f, -0.5f, -0.5f}, // 1
        { 0.5f,  0.5f, -0.5f}, // 2
        {-0.5f,  0.5f, -0.5f}, // 3
        {-0.5f, -0.5f,  0.5f}, // 4
        { 0.5f, -0.5f,  0.5f}, // 5
        { 0.5f,  0.5f,  0.5f}, // 6
        {-0.5f,  0.5f,  0.5f}  // 7
    };

    // Define face normals
    Vec3f faceNormals[] = {
        { 0.f,  0.f, -1.f}, // Front
        { 0.f,  0.f,  1.f}, // Back
        { 0.f, -1.f,  0.f}, // Bottom
        { 0.f,  1.f,  0.f}, // Top
        {-1.f,  0.f,  0.f}, // Left
        { 1.f,  0.f,  0.f}  // Right
    };

    // Define the triangles for each face using vertex indices
    int faceIndices[][6] = {
        {0, 1, 2, 2, 3, 0}, // Front
        {5, 4, 7, 7, 6, 5}, // Back
        {0, 4, 5, 5, 1, 0}, // Bottom
        {3, 2, 6, 6, 7, 3}, // Top
        {4, 0, 3, 3, 7, 4}, // Left
        {1, 5, 6, 6, 2, 1}  // Right
    };

    std::vector<Vec3f> pos;
    std::vector<Vec3f> normals;

    // Populate positions and normals
    for (int face = 0; face < 6; ++face) {
        for (int i = 0; i < 6; ++i) {
            pos.emplace_back(vertices[faceIndices[face][i]]);
            normals.emplace_back(normalize(N * faceNormals[face]));
        }
    }

    // Apply the transformation to the positions
    for (auto& p : pos) {
        Vec4f p4{ p.x, p.y, p.z, 1.f };
        Vec4f t = aPreTransform * p4;

        t /= t.w;  // Perspective division
        p = Vec3f{ t.x, t.y, t.z };
    }

    std::vector<Vec2f> texcoords;
    std::vector<int> material_ids(pos.size(), 0); 
    std::vector<Material> materials(1, aMaterial);

    return SimpleMeshData{ 
        std::move(pos), 
        std::move(texcoords),
        std::move(normals),
        std::move(material_ids),
        std::move(materials) 
    };
}
