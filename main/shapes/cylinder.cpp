#include "cylinder.hpp"

#include <numbers>

SimpleMeshData make_cylinder(bool aCapped, std::size_t aSubdivs, Material aMaterial, Mat44f aPreTransform) 
{
    // Calculate normal matrix
    Mat33f const N = mat44_to_mat33(transpose(invert(aPreTransform)));

    std::vector<Vec3f> pos;
    std::vector<Vec3f> normals;

    float prevY = std::cos(0.f);
    float prevZ = std::sin(0.f);

    for (std::size_t i = 0; i < aSubdivs; ++i) {
        float const angle = (i + 1) / float(aSubdivs) * 2.f * std::numbers::pi_v<float>;
        float y = std::cos(angle);
        float z = std::sin(angle);

        // Normals for the side faces
        Vec3f n1 = { 0.f, prevY, prevZ };
        Vec3f n2 = { 0.f, y, z };

        // First triangle (side)
        pos.emplace_back(Vec3f{ 0.f, prevY, prevZ });
        normals.emplace_back(normalize(N * n1));

        pos.emplace_back(Vec3f{ 0.f, y, z });
        normals.emplace_back(normalize(N * n2));

        pos.emplace_back(Vec3f{ 1.f, prevY, prevZ });
        normals.emplace_back(normalize(N * n1));

        // Second triangle (side)
        pos.emplace_back(Vec3f{ 0.f, y, z });
        normals.emplace_back(normalize(N * n2));

        pos.emplace_back(Vec3f{ 1.f, y, z });
        normals.emplace_back(normalize(N * n2));

        pos.emplace_back(Vec3f{ 1.f, prevY, prevZ });
        normals.emplace_back(normalize(N * n1));


        if (!aCapped) {
            prevY = y;
            prevZ = z;
            continue;
        }

        // Normals for caps
        Vec3f capNormalRight = { 1.f, 0.f, 0.f };
        Vec3f capNormalLeft = { -1.f, 0.f, 0.f };

        // Right cap
        pos.emplace_back(Vec3f{ 1.f, 0.f, 0.f });
        normals.emplace_back(normalize(N * capNormalRight));

        pos.emplace_back(Vec3f{ 1.f, prevY, prevZ });
        normals.emplace_back(normalize(N * capNormalRight));

        pos.emplace_back(Vec3f{ 1.f, y, z });
        normals.emplace_back(normalize(N * capNormalRight));

        // Left cap
        pos.emplace_back(Vec3f{ 0.f, 0.f, 0.f });
        normals.emplace_back(normalize(N * capNormalLeft));

        pos.emplace_back(Vec3f{ 0.f, y, z });
        normals.emplace_back(normalize(N * capNormalLeft));

        pos.emplace_back(Vec3f{ 0.f, prevY, prevZ });
        normals.emplace_back(normalize(N * capNormalLeft));

        prevY = y;
        prevZ = z;

    }

    // Apply perspective transformation to positions
    for (auto& p : pos) {
        Vec4f p4{ p.x, p.y, p.z, 1.f };
        Vec4f t = aPreTransform * p4;

        t /= t.w;  // Perspective division
        p = Vec3f{ t.x, t.y, t.z };
    }

    std::vector<Vec2f> texcoords;
    std::vector<int> material_ids(pos.size(), 0); 

    return SimpleMeshData{ 
        std::move(pos), 
        std::move(texcoords),
        std::move(normals),
        std::move(material_ids),
        { aMaterial }
    };
}