#include "simple_mesh.hpp"

SimpleMeshData concatenate( SimpleMeshData aM, SimpleMeshData const& aN )
{
	aM.positions.insert( aM.positions.end(), aN.positions.begin(), aN.positions.end() );
    aM.texcoords.insert( aM.texcoords.end(), aN.texcoords.begin(), aN.texcoords.end() );
    aM.normals.insert( aM.normals.end(), aN.normals.begin(), aN.normals.end() );

    // Concatenate materials
    // This ivalidates material_ids
    aM.materials.insert(aM.materials.end(), aN.materials.begin(), aN.materials.end());

    // Calculate the offset for material_ids from aN
    std::size_t materialOffset = aM.materials.size() - aN.materials.size();

    // Concatenate material_ids with adjustment
    std::transform(
        aN.material_ids.begin(),    // Start of source
        aN.material_ids.end(),      // End of source range
        std::back_inserter(aM.material_ids),    // output iterator that appends to aM
        [materialOffset](std::size_t id) { return id + materialOffset; }    // Lambda function that adapts IDs
    );

	return aM;
}



GLuint create_vao( SimpleMeshData &aMeshData )
{
    // Add defaults to the mesh if needed
    if (aMeshData.texcoords.empty()) {
        std::vector<Vec2f> defaultTexcoords(aMeshData.positions.size(), Vec2f{0.0f, 0.0f});
        aMeshData.texcoords = std::move(defaultTexcoords);
    }

    // Generate object buffers for positions
    GLuint positionVBO = 0;
    glGenBuffers( 1, &positionVBO );

    glBindBuffer( GL_ARRAY_BUFFER, positionVBO );
    glBufferData( 
        GL_ARRAY_BUFFER, 
        aMeshData.positions.size() * sizeof(Vec3f),
        aMeshData.positions.data(),     // This gives us the raw pointer to underlying data
        GL_STATIC_DRAW 
    );

    // Do the same for colors
    GLuint texCoordVBO = 0;
    glGenBuffers( 1, &texCoordVBO );

    glBindBuffer( GL_ARRAY_BUFFER, texCoordVBO );
    glBufferData( 
        GL_ARRAY_BUFFER, 
        aMeshData.texcoords.size() * sizeof(Vec2f),
        aMeshData.texcoords.data(),
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

    // Texture coordinates
    glBindBuffer( GL_ARRAY_BUFFER, texCoordVBO );
    glVertexAttribPointer(
        2,
        2, GL_FLOAT, GL_FALSE,
        0,
        nullptr
    );
    glEnableVertexAttribArray( 2 );


    // === Materials ===
    // Then we add each of the material attributes, using respective offsets
    glBindBuffer( GL_ARRAY_BUFFER, materialVBO );

    glVertexAttribPointer(
        3,
        3, GL_FLOAT, GL_FALSE,
        sizeof(Material),                   // Stride
        (void*)offsetof(Material, ambient)  // The offset of ambient within Material
    );
    glEnableVertexAttribArray( 3 );

    // Diffuse
    glVertexAttribPointer(
        4,
        3, GL_FLOAT, GL_FALSE,
        sizeof(Material),
        (void*)offsetof(Material, diffuse)
    );
    glEnableVertexAttribArray( 4 );

    // Specular
    glVertexAttribPointer(
        5,
        3, GL_FLOAT, GL_FALSE,
        sizeof(Material),
        (void*)offsetof(Material, specular)
    );
    glEnableVertexAttribArray( 5 );

    // Shininess
    glVertexAttribPointer(
        6,
        1, GL_FLOAT, GL_FALSE,
        sizeof(Material),
        (void*)offsetof(Material, shininess)
    );
    glEnableVertexAttribArray( 6 );

    // Emissive
    glVertexAttribPointer(
        7,
        3, GL_FLOAT, GL_FALSE,
        sizeof(Material),
        (void*)offsetof(Material, emissive)
    );
    glEnableVertexAttribArray( 7 );

    // Illum
    glVertexAttribPointer(
        8,
        1, GL_FLOAT, GL_FALSE,
        sizeof(Material),
        (void*)offsetof(Material, illum)
    );
    glEnableVertexAttribArray( 8 );

    // Cleanup
    glBindVertexArray( 0 );
    glBindBuffer( GL_ARRAY_BUFFER, 0 );

    // Delete buffers
    glDeleteBuffers(1, &positionVBO);
    glDeleteBuffers(1, &texCoordVBO);
    glDeleteBuffers(1, &normalsVBO);
    glDeleteBuffers(1, &materialVBO);

    return vao;
}

