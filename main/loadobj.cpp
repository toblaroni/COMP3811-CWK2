#include "loadobj.hpp"

#include <rapidobj/rapidobj.hpp>

#include "../support/error.hpp"

SimpleMeshData load_wavefront_obj( char const* aPath )
{
	auto result = rapidobj::ParseFile( aPath );

    if (result.error) 
        throw Error(
            "Unable to load OBJ file '%s': '%s'.", 
            aPath, result.error.code.message().c_str()
        );

    // Triangulate any faces that aren't triangles
    rapidobj::Triangulate( result );

    SimpleMeshData ret;

    // Store materials
    for (const auto& mat : result.materials) {
        ret.materials.emplace_back( Material {
            Vec3f{ mat.ambient[0], mat.ambient[1], mat.ambient[2] },
            Vec3f{ mat.diffuse[0], mat.diffuse[1], mat.diffuse[2] },
            Vec3f{ mat.specular[0], mat.specular[1], mat.specular[2] },
            mat.shininess,
            Vec3f{ mat.emission[0], mat.emission[1], mat.emission[2] },
            static_cast<float>(mat.illum)
        });
    }

    // Geometry
    for (auto const& shape : result.shapes) {
        for (std::size_t i = 0; i < shape.mesh.indices.size(); ++i) {
            auto const& idx = shape.mesh.indices[i];

            ret.positions.emplace_back( Vec3f {
                result.attributes.positions[idx.position_index*3+0],
                result.attributes.positions[idx.position_index*3+1],
                result.attributes.positions[idx.position_index*3+2]
            } );

            ret.normals.emplace_back( Vec3f {
                result.attributes.normals[idx.normal_index*3+0],    // Use normal index 
                result.attributes.normals[idx.normal_index*3+1],
                result.attributes.normals[idx.normal_index*3+2]
            });

            // Each shape has a material ID
            std::size_t material_id = shape.mesh.material_ids[i / 3];
            ret.material_ids.emplace_back( material_id );
        }
    }

    return ret;
}

