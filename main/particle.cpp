// https://learnopengl.com/index.php?p=In-Practice/2D-Game/Particles

#include "particle.hpp"

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

void ParticleSystem::draw( GLuint uProjCameraWorldLocation, Mat44f projCameraWorld ) {
    /*
     *  GL_ONE allows 'additive blending', which gives us the glow effect when 
     *  sprites are stacked on each other
     */

    glBlendFunc( GL_SRC_ALPHA, GL_ONE );
    glUseProgram( this->shader.programId() );

    glBindVertexArray(this->vao);
    glBindTexture(GL_TEXTURE_2D, this->textureId);

    for (Particle particle : this->particles) {
        if (!particle.isDead()) {
            glUniform3fv( this->uOffsetLocation, 1, &particle.position.x );
            glUniform3fv( this->uColorLocation, 1, &particle.color.x );
            glUniformMatrix4fv(uProjCameraWorldLocation, 1, GL_TRUE, projCameraWorld.v);
            glDrawArrays( GL_TRIANGLES, 0, 6 );
        }
    }

    glBindVertexArray( 0 );
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);  // Reset blending
    glUseProgram( 0 );
}
