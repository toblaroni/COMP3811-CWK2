#include "cone.hpp"

#include <numbers>

SimpleMeshData make_cone( bool aCapped, std::size_t aSubdivs, Material aMaterial, Mat44f aPreTransform )
{
    // Calculate normal matrix
    Mat33f const N = mat44_to_mat33(transpose(invert(aPreTransform)));

	std::vector<Vec3f> pos;
    std::vector<Vec3f> normals;

    float prevY = std::cos( 0.f );
    float prevZ = std::sin( 0.f );

    for ( std::size_t i = 0; i < aSubdivs; ++i ) {
        float const angle = (i+1) / float(aSubdivs) * 2.f * std::numbers::pi_v<float>;
        float y = std::cos( angle );
        float z = std::sin( angle );

        pos.emplace_back( Vec3f{ 0.f, prevY, prevZ } );
        pos.emplace_back( Vec3f{ 0.f, y, z } );
        pos.emplace_back( Vec3f{ 1.f, 0.f, 0.f } );

        normals.emplace_back(normalize(N * Vec3f{ 0.f, prevY, prevZ }));
        normals.emplace_back(normalize(N * Vec3f{ 0.f, y, z } ));
        normals.emplace_back(normalize(N * Vec3f{ 1.f, 0.f, 0.f } ));

        prevY = y;
        prevZ = z;

        if (!aCapped) continue;

        // Draw cap
        pos.emplace_back( Vec3f{ 0.f, prevY, prevZ } );
        pos.emplace_back( Vec3f{ 0.f, y, z } );
        pos.emplace_back( Vec3f{ 0.f, 0.f, 0.f } );

        normals.emplace_back(normalize(N * Vec3f{ 0.f, prevY, prevZ }));
        normals.emplace_back(normalize(N * Vec3f{ 0.f, y, z } ));
        normals.emplace_back(normalize(N * Vec3f{ 0.f, 0.f, 0.f } ));
    }

    for ( auto& p : pos ) {
        Vec4f p4 { p.x, p.y, p.z, 1.f };
        Vec4f t = aPreTransform * p4;

        t /= t.w;   // This changes perspective I think

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

