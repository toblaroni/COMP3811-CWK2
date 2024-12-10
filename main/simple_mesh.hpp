#ifndef SIMPLE_MESH_HPP_C6B749D6_C83B_434C_9E58_F05FC27FEFC9
#define SIMPLE_MESH_HPP_C6B749D6_C83B_434C_9E58_F05FC27FEFC9

#include <glad/glad.h>

#include <vector>

#include "../vmlib/vec2.hpp"
#include "../vmlib/vec3.hpp"

struct Material {
	Vec3f ambient;		// Ka
	Vec3f diffuse;		// Kd
	Vec3f specular;		// Ks
	float shininess;	// Ns
	Vec3f emissive;		// Ke
	float illum;		// Illumination model
};

struct SimpleMeshData
{
	std::vector<Vec3f> positions;
	std::vector<Vec2f> texcoords;
	std::vector<Vec3f> normals;
	std::vector<int> material_ids;
	std::vector<Material> materials;
};

SimpleMeshData concatenate( SimpleMeshData, SimpleMeshData const& );

GLuint create_vao( SimpleMeshData& );

#endif // SIMPLE_MESH_HPP_C6B749D6_C83B_434C_9E58_F05FC27FEFC9
