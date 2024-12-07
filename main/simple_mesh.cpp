#include "simple_mesh.hpp"

SimpleMeshData concatenate( SimpleMeshData aM, SimpleMeshData const& aN )
{
	aM.positions.insert( aM.positions.end(), aN.positions.begin(), aN.positions.end() );
	aM.colors.insert( aM.colors.end(), aN.colors.begin(), aN.colors.end() );
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

    // Do the same for colors
    GLuint colorVBO = 0;
    glGenBuffers( 1, &colorVBO );

    glBindBuffer( GL_ARRAY_BUFFER, colorVBO );
    glBufferData( 
        GL_ARRAY_BUFFER, 
        aMeshData.colors.size() * sizeof(Vec3f),
        aMeshData.colors.data(),
        GL_STATIC_DRAW 
    );

    // Generate VAO, define attributes
    GLuint vao = 0;
    glGenVertexArrays( 1, &vao );
    glBindVertexArray( vao );

    glBindBuffer( GL_ARRAY_BUFFER, positionVBO );
    glVertexAttribPointer(
        0,
        3, GL_FLOAT, GL_FALSE,
        0,
        0
    );
    glEnableVertexAttribArray( 0 );

    glBindBuffer( GL_ARRAY_BUFFER, colorVBO );
    glVertexAttribPointer(
        1,
        3, GL_FLOAT, GL_FALSE,
        0,
        0
    );
    glEnableVertexAttribArray( 1 );

    // Cleanup
    glBindVertexArray( 0 );
    glBindBuffer( GL_ARRAY_BUFFER, 0 );

    return vao;
}

