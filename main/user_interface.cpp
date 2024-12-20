#include "user_interface.hpp"




GLuint create_UI_vao( UserInterface &aUI )
{

    std::vector<Vec2f> positions;
    std::vector<Vec4f> colors;

    for (auto b: aUI.buttons) {

        // Button Fill
        positions.push_back(b.corner2); // Bottom Right
        positions.push_back({b.corner2.x, b.corner1.y}); // Bottom Left
        positions.push_back(b.corner1); // Top Left

        positions.push_back(b.corner2); // Bottom Right
        positions.push_back(b.corner1); // Top Left
        positions.push_back({b.corner1.x, b.corner2.y}); // Top Right

        // -------------------------------------------

        // Outline: Top Edge
        positions.push_back({b.corner1.x, b.corner1.y}); // Outer Top Left
        positions.push_back({b.corner2.x - 0.01f, b.corner1.y - 0.01f}); // Inner Top Right
        positions.push_back({b.corner2.x, b.corner1.y}); // Outer Top Right

        positions.push_back({b.corner1.x, b.corner1.y}); // Outer Top Left
        positions.push_back({b.corner1.x + 0.01f, b.corner1.y - 0.01f}); // Inner Top Left
        positions.push_back({b.corner2.x - 0.01f, b.corner1.y - 0.01f}); // Inner Top Right

        // Outline: Bottom Edge
        positions.push_back({b.corner1.x, b.corner2.y}); // Outer Bottom Left
        positions.push_back({b.corner2.x, b.corner2.y}); // Outer Bottom Right
        positions.push_back({b.corner2.x - 0.01f, b.corner2.y + 0.01f}); // Inner Bottom Right

        positions.push_back({b.corner1.x, b.corner2.y}); // Outer Bottom Left
        positions.push_back({b.corner2.x - 0.01f, b.corner2.y + 0.01f}); // Inner Bottom Right
        positions.push_back({b.corner1.x + 0.01f, b.corner2.y + 0.01f}); // Inner Bottom Left

        // Outline: Left Edge
        positions.push_back({b.corner1.x, b.corner1.y}); // Outer Top Left
        positions.push_back({b.corner1.x, b.corner2.y}); // Outer Bottom Left
        positions.push_back({b.corner1.x + 0.01f, b.corner2.y + 0.01f}); // Inner Bottom Left

        positions.push_back({b.corner1.x + 0.01f, b.corner1.y - 0.01f}); // Inner Top Left
        positions.push_back({b.corner1.x, b.corner1.y}); // Outer Top Left
        positions.push_back({b.corner1.x + 0.01f, b.corner2.y + 0.01f}); // Inner Bottom Left

        // Outline: Right Edge
        positions.push_back({b.corner2.x, b.corner1.y}); // Outer Top Right
        positions.push_back({b.corner2.x - 0.01f, b.corner2.y + 0.01f}); // Inner Bottom Right
        positions.push_back({b.corner2.x, b.corner2.y}); // Outer Bottom Right

        positions.push_back({b.corner2.x, b.corner1.y}); // Outer Top Right
        positions.push_back({b.corner2.x - 0.01f, b.corner1.y - 0.01f}); // Inner Top Right
        positions.push_back({b.corner2.x - 0.01f, b.corner2.y + 0.01f}); // Inner Bottom Righ


        // Add colors
        for (int i = 0; i < 30; ++i) {
            colors.push_back(b.color);
        }

    }


    // Generate object buffers for positions
    GLuint positionVBO = 0;
    glGenBuffers( 1, &positionVBO );

    glBindBuffer( GL_ARRAY_BUFFER, positionVBO );
    glBufferData( 
        GL_ARRAY_BUFFER, 
        positions.size() * sizeof(Vec2f),
        positions.data(),     // This gives us the raw pointer to underlying data
        GL_STATIC_DRAW 
    );


    // Do the same for colors
    GLuint colorVBO = 0;
    glGenBuffers( 1, &colorVBO );

    glBindBuffer( GL_ARRAY_BUFFER, colorVBO );
    glBufferData( 
        GL_ARRAY_BUFFER, 
        colors.size() * sizeof(Vec4f),
        colors.data(),
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
        2, GL_FLOAT, GL_FALSE,
        0,
        0
    );
    glEnableVertexAttribArray( 0 );

    glBindBuffer( GL_ARRAY_BUFFER, colorVBO ); 
    glVertexAttribPointer(
        1,
        4, GL_FLOAT, GL_FALSE,
        0,
        0
    );
    glEnableVertexAttribArray( 1 );


    // Cleanup
    glBindVertexArray( 0 );
    glBindBuffer( GL_ARRAY_BUFFER, 0 );

    // Delete buffers
    glDeleteBuffers(1, &positionVBO);
    glDeleteBuffers(1, &colorVBO);


    return vao;
}

