#include "simple_mesh.hpp"
#include <cstddef>

SimpleMeshData concatenate( SimpleMeshData aM, SimpleMeshData const& aN )
{
	aM.positions.insert( aM.positions.end(), aN.positions.begin(), aN.positions.end() );
    aM.normals.insert( aM.normals.end(), aN.normals.begin(), aN.normals.end() );
    // Adding materials here ? not sure we need it
	return aM;
}


GLuint create_vao( SimpleMeshData const& aMeshData )
{
    // Generate object buffers for positions
    GLuint positionVBO = 0;
    glGenBuffers( 1, &positionVBO );

    // Then we need to bind the buffer
    glBindBuffer( GL_ARRAY_BUFFER, positionVBO );
    glBufferData( 
        GL_ARRAY_BUFFER, 
        aMeshData.positions.size() * sizeof(Vec3f),
        aMeshData.positions.data(),     // This gives us the raw pointer to underlying data
        GL_STATIC_DRAW 
    );

    // Normals
    GLuint normalsVBO = 0;
    glGenBuffers( 1, &normalsVBO );

    glBindBuffer( GL_ARRAY_BUFFER, normalsVBO );
    glBufferData(
        GL_ARRAY_BUFFER,
        aMeshData.normals.size() * sizeof(Vec3f),
        aMeshData.normals.data(),
        GL_STATIC_DRAW
    );

    // Material properties 
    GLuint materialVBO = 0;
    glGenBuffers( 1, &materialVBO );

    // get the material properties into a single array for each vertex
    std::vector<Material> materials(aMeshData.material_ids.size());
    for (std::size_t i = 0; i < aMeshData.material_ids.size(); ++i) {
        // Copy the material for each vertex
        std::size_t material_id = aMeshData.material_ids[i];
        materials[i] = aMeshData.materials[material_id];
    }

    glBindBuffer( GL_ARRAY_BUFFER, materialVBO );
    glBufferData(
        GL_ARRAY_BUFFER,
        materials.size() * sizeof(Material),
        materials.data(),
        GL_STATIC_DRAW
    );

    // Generate VAO, define attributes
    GLuint vao = 0;
    glGenVertexArrays( 1, &vao );
    glBindVertexArray( vao );

    // Add positions
    glBindBuffer( GL_ARRAY_BUFFER, positionVBO );
    glVertexAttribPointer(
        0,
        3, GL_FLOAT, GL_FALSE,
        0,
        0
    );
    glEnableVertexAttribArray( 0 );

    // Add normals
    glBindBuffer( GL_ARRAY_BUFFER, normalsVBO );
    glVertexAttribPointer(
        1,
        3, GL_FLOAT, GL_FALSE,
        0,
        0
    );
    glEnableVertexAttribArray( 1 );

    // === Materials ===
    // Then we add each of the material attributes, using respective offsets
    glBindBuffer( GL_ARRAY_BUFFER, materialVBO );

    // Ambient
    glVertexAttribPointer(
        2,
        3, GL_FLOAT, GL_FALSE,
        sizeof(Material),                   // Stride
        (void*)offsetof(Material, ambient)  // The offset of ambient within Material
    );
    glEnableVertexAttribArray( 2 );

    // Diffuse
    glVertexAttribPointer(
        3,
        3, GL_FLOAT, GL_FALSE,
        sizeof(Material),
        (void*)offsetof(Material, diffuse)
    );
    glEnableVertexAttribArray( 3 );

    // Specular
    glVertexAttribPointer(
        4,
        3, GL_FLOAT, GL_FALSE,
        sizeof(Material),
        (void*)offsetof(Material, specular)
    );
    glEnableVertexAttribArray( 4 );

    // Shininess
    glVertexAttribPointer(
        5,
        1, GL_FLOAT, GL_FALSE,
        sizeof(Material),
        (void*)offsetof(Material, shininess)
    );
    glEnableVertexAttribArray( 5 );

    // Emissive
    glVertexAttribPointer(
        6,
        3, GL_FLOAT, GL_FALSE,
        sizeof(Material),
        (void*)offsetof(Material, emissive)
    );
    glEnableVertexAttribArray( 6 );

    // Illum
    glVertexAttribPointer(
        7,
        1, GL_FLOAT, GL_FALSE,
        sizeof(Material),
        (void*)offsetof(Material, illum)
    );
    glEnableVertexAttribArray( 7 );

    // Cleanup
    glBindVertexArray( 0 );
    glBindBuffer( GL_ARRAY_BUFFER, 0 );

    // Delete buffers??

    return vao;
}

