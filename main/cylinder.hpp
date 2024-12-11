#ifndef CYLINDER_HPP_E4D1E8EC_6CDA_4800_ABDD_264F643AF5DB
#define CYLINDER_HPP_E4D1E8EC_6CDA_4800_ABDD_264F643AF5DB

#include <vector>

#include <cstdlib>

#include "simple_mesh.hpp"

#include "../vmlib/vec3.hpp"
#include "../vmlib/mat44.hpp"
#include "../vmlib/mat33.hpp"


SimpleMeshData make_cylinder(
	bool aCapped = true,
	std::size_t aSubdivs = 16,
    Material defaultMaterial = { {0.1f, 0.1f, 0.1f},    // Ambience
                                 {0.8f, 0.8f, 0.8f},    // Diffuse
                                 {0.5f, 0.5f, 0.5f},    // Specular
                                 15.0f,                 // Shininess
                                 {0.0f, 0.0f, 0.0f},    // Emissive
                                 1.0f },                // Illum
	Mat44f aPreTransform = kIdentity44f
);

#endif // CYLINDER_HPP_E4D1E8EC_6CDA_4800_ABDD_264F643AF5DB
