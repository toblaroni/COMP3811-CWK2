// https://learnopengl.com/index.php?p=In-Practice/2D-Game/Particles

#include "particle.hpp"
#include <iostream>

ParticleSystem::ParticleSystem( ShaderProgram& shader, GLuint textureId, unsigned int amount) 
    : shader(shader), 
      textureId(textureId), 
      numParticles(amount),
      lastUsedParticle(0)
{
    this->init();
}

void ParticleSystem::update( float dt, Vec3f objPosition, Vec3f objVelocity, unsigned int numNewParticles ) 
{
    for (unsigned int i = 0; i < numNewParticles; ++i) {
        int unusedParticle = this->firstUnusedParticle();
        this->respawnParticle( this->particles[unusedParticle], objPosition, objVelocity );
    }

    // Update particles
    for (unsigned int i = 0; i < this->numParticles; ++i) {
        Particle &p = this->particles[i];
        p.lifetime -= dt;

        if ( !p.isDead() ) {
            p.position -= p.velocity * dt;
            p.color.w  -= dt * 2.5;         // Decrement alpha
        }
    }
}

void ParticleSystem::init() {
    // Generate the uniform locations
    this->uOffsetLocation = glGetUniformLocation( this->shader.programId(), "uOffset" );
    this->uColorLocation  = glGetUniformLocation( this->shader.programId(), "uColor" );
    this->uProjCameraWorldLocation = glGetUniformLocation( this->shader.programId(), "uProjCameraWorld" );

    if (this->uOffsetLocation == -1) 
        std::fprintf(stderr, "Error: 'uOffsetLocation' not found");
    if (this->uColorLocation == -1) 
        std::fprintf(stderr, "Error: 'uColorLocation' not found");
    if (this->uProjCameraWorldLocation == -1) 
        std::fprintf(stderr, "Error: 'uProjCameraWorldLocation' not found");

    // Vertex position (CCW)
    float positions[] = {
        0.0f, 1.0f, 0.0f,   // Top-left
        0.0f, 0.0f, 0.0f,   // Bottom-left
        1.0f, 0.0f, 0.0f,   // Bottom-right

        1.0f, 1.0f, 0.0f,   // Top-right
        0.0f, 1.0f, 0.0f,   // Top-left
        1.0f, 0.0f, 0.0f    // Bottom-right
    };

    // Texture coordinates
    float texCoords[] = {
        0.0f, 1.0f,   // Top-left
        0.0f, 0.0f,   // Bottom-left
        1.0f, 0.0f,   // Bottom-right

        1.0f, 1.0f,   // Top-right
        0.0f, 1.0f,   // Top-left
        1.0f, 0.0f    // Bottom-right
    };

    // Position VBO
    GLuint positionVBO = 0;
    glGenBuffers( 1, &positionVBO );
    glBindBuffer(GL_ARRAY_BUFFER, positionVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(positions), positions, GL_STATIC_DRAW);

    // Texture VBO
    GLuint textureVBO = 0;
    glGenBuffers( 1, &textureVBO );
    glBindBuffer(GL_ARRAY_BUFFER, textureVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(texCoords), texCoords, GL_STATIC_DRAW);
    
    // VAO
    glGenVertexArrays( 1, &this->vao );
    glBindVertexArray( this->vao );

    // Add position attributes
    glBindBuffer( GL_ARRAY_BUFFER, positionVBO );
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
    glEnableVertexAttribArray( 0 );

    // Add texture coordinates
    glBindBuffer( GL_ARRAY_BUFFER, textureVBO );
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), nullptr);
    glEnableVertexAttribArray( 1 );
    
    // Clean up
    glBindBuffer( GL_ARRAY_BUFFER, 0 );
    glBindVertexArray( 0 );
    glDeleteBuffers( 1, &positionVBO );
    glDeleteBuffers( 1, &textureVBO );

    for (unsigned int i = 0; i < this->numParticles; ++i)
        this->particles.push_back(Particle());
}

unsigned int ParticleSystem::firstUnusedParticle() {
    // Search from last used particle
    for (unsigned int i = this->lastUsedParticle; i < this->numParticles; ++i) {
        if (this->particles[i].isDead()) {
            this->lastUsedParticle = i;
            return i;
        }
    }

    // linear seach
    for (unsigned int i = 0; i < this->lastUsedParticle; ++i ) {
        if ( this->particles[i].isDead() ) {
            this->lastUsedParticle = i;
            return i;
        }
    }

    // reset
    this->lastUsedParticle = 0;
    return 0;
}

void ParticleSystem::respawnParticle( Particle& particle, Vec3f objPosition, Vec3f objVelocity ) {
    float random = ((rand() % 100) - 50) / 10.f;
    float rColor = 0.5f + ((rand() % 100) / 100.f); // Separate into rgb?

    particle.position = objPosition + Vec3f{random, random, random};
    particle.color = {rColor, rColor, rColor, 1.f};
    particle.lifetime = 1.f;
    particle.velocity = objVelocity * 0.1f;
}

// Create here as not used anywhere else
Mat44f createBillboardRotationMatrix( const Vec3f& toCamera ) {
    Vec3f right = normalize( cross( { 0.f, 1.f, 0.f }, toCamera ) );

    Vec3f up = normalize( cross( toCamera, right ) );

    // Construct the rotation matrix (camera-facing)
    Mat44f rotation;

    // Set the first row as the right vector
    rotation(0, 0) = right.x;
    rotation(0, 1) = right.y;
    rotation(0, 2) = right.z;
    rotation(0, 3) = 0.0f;  // The last element of the row is set to 0 for direction

    // Set the second row as the up vector
    rotation(1, 0) = up.x;
    rotation(1, 1) = up.y;
    rotation(1, 2) = up.z;
    rotation(1, 3) = 0.0f;

    // Set the third row as the camera-facing vector
    rotation(2, 0) = toCamera.x;
    rotation(2, 1) = toCamera.y;
    rotation(2, 2) = toCamera.z;
    rotation(2, 3) = 0.0f;

    // The fourth row is the default for homogeneous coordinates (translation)
    rotation(3, 0) = 0.0f;
    rotation(3, 1) = 0.0f;
    rotation(3, 2) = 0.0f;
    rotation(3, 3) = 1.0f;  // Set the last element to 1 for homogeneous coordinates

    return rotation;
}

void ParticleSystem::draw( Mat44f projCameraWorld, Mat44f viewMatrix ) {
    /*
     *  GL_ONE allows 'additive blending', which gives us the glow effect when 
     *  sprites are stacked on each other
     */
    glBlendFunc( GL_SRC_ALPHA, GL_ONE );

    glUseProgram( this->shader.programId() );

    glBindVertexArray(this->vao);

    glActiveTexture( GL_TEXTURE0 );
    glBindTexture(GL_TEXTURE_2D, this->textureId);

    Mat44f viewMatrixInverse = invert( viewMatrix );

    for (Particle particle : this->particles) {
        if (!particle.isDead()) {
            // Billboarding
            Vec4f particlePositionHomo = Vec4f { particle.position.x, particle.position.y, particle.position.z, 1.0f };
            Vec4f transformedPosition = viewMatrixInverse * particlePositionHomo;
            Vec3f particlePositionInCameraSpace = Vec3f { transformedPosition.x, transformedPosition.y, transformedPosition.z };

            Mat44f rotationMatrix = createBillboardRotationMatrix( particlePositionInCameraSpace );
            Mat44f modelMatrix = rotationMatrix * make_translation( particle.position );

            // This may be expensive
            glUniform3fv( this->uOffsetLocation, 1, modelMatrix.v );

            glUniform4fv( this->uColorLocation, 1, &particle.color.x );

            glUniformMatrix4fv(this->uProjCameraWorldLocation, 1, GL_TRUE, projCameraWorld.v);

            glDrawArrays( GL_TRIANGLES, 0, 6 );
        }
    }

    // Clean up
    glBindVertexArray( 0 );
    glBindTexture(GL_TEXTURE_2D, 0);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);  // Reset blending
    glUseProgram( 0 );
}
