// https://learnopengl.com/index.php?p=In-Practice/2D-Game/Particles

#include "particle.hpp"
#include <cstdio>

ParticleSystem::ParticleSystem( ShaderProgram& shader, GLuint textureId, unsigned int amount)
    : shader(shader),
      textureId(textureId),
      numParticles(amount),
      lastUsedParticle(0)
{
    this->init();
}

// https://en.wikipedia.org/wiki/Smoothstep
float smoothstep(float edge0, float edge1, float x) {
    float t = std::clamp((x - edge0) / (edge1 - edge0), 0.0f, 1.0f);
    return t * t * (3.0 - 2.0 * t);
}


void ParticleSystem::update( float dt, Vec3f objPosition, Vec3f objVelocity, unsigned int numNewParticles, Vec3f cameraPos )
{

    for (unsigned int i = 0; i < numNewParticles; ++i) {
        int unusedParticle = this->firstUnusedParticle();
        this->respawnParticle( this->particles[unusedParticle], objPosition, objVelocity );
    }

    // Update particles
    for (unsigned int i = 0; i < this->numParticles; ++i) {
        Particle &p = this->particles[i];

        if (p.isDead()) continue;

        p.lifetime -= dt * 4.f;

        if ( !p.isDead() ) {
            p.position += p.velocity * dt;
            p.color.w = smoothstep(0.0f, 1.0f, p.lifetime);     // Fade out over the lifetime

            // Make the particle smaller over its lifetime
            float progress = 1.0f - p.lifetime; // 0 at start, 1 at end
            p.size = smoothstep(1.0f, 0.0f, progress) * p.initialSize; // Interpolated size
        }  
    }
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
    float randomX = ((rand() % 100) - 50) / 100.f;
    float randomY = ((rand() % 100) - 50) / 100.f;
    float randomZ = ((rand() % 100) - 50) / 100.f;

    float rColor = 0.5f + ((rand() % 100) / 100.f); // Separate into rgb?

    // particle.position = objPosition + Vec3f{randomX * radius, randomY * radius, randomZ * radius};
    particle.position = objPosition;
    particle.color = {rColor, rColor, rColor, 1.f};
    particle.lifetime = 1.f;
    particle.initialSize =  0.1f + ((rand() % 30) / 100.f);

    // Random velocity for the particle to shoot out in a random direction
    float randomVelocityFactor = 0.1f + ((rand() % 100) / 100.f);  // Random factor for velocity strength

    // Easily adjust the spread of the particles
    float radius = 2.f;

    // Here we want minus velocity so that particles shoot the opposite way to the rocket.
    particle.velocity = (objVelocity*-0.5f) + Vec3f{randomX*radius, randomY*radius, randomZ*radius} * randomVelocityFactor;
}

void ParticleSystem::reset( Vec3f objPosition ) {
    for (unsigned int i = 0; i < this->numParticles; ++i) {
        this->particles[i].position = objPosition;
        this->particles[i].velocity = Vec3f { 0.f, 0.f, 0.f };
    }
}

// Re calculates distances from camera and sorts
void ParticleSystem::orderParticles( Mat44f projCameraWorld ) {
    for (Particle& p : this->particles) {
        if (p.isDead()) {
            p.ndcDepth = -1;
            continue;
        }

        // Use Clip space to order rather than world space
        // https://chatgpt.com/
        Vec4f clipSpacePos = projCameraWorld * Vec4f{ p.position.x, p.position.y, p.position.z, 1.f };

        p.ndcDepth = clipSpacePos.z / clipSpacePos.w;     // NDC

   }

    sortParticles();
}

void ParticleSystem::init() {
    // Generate the uniform locations
    this->uColorLocation  = glGetUniformLocation( this->shader.programId(), "uColor" );
    this->uProjCameraWorldLocation = glGetUniformLocation( this->shader.programId(), "uProjCameraWorld" );

    this->uParticleCenterWorldspaceLocation = glGetUniformLocation( this->shader.programId(), "uParticleCenterWorldSpace" );
    this->uCameraRightWorldSpaceLocation = glGetUniformLocation( this->shader.programId(), "uCameraRightWorldSpace" );
    this->uCameraUpWorldSpaceLocation = glGetUniformLocation( this->shader.programId(), "uCameraUpWorldSpace" );
    this->uBillboardSizeLocation = glGetUniformLocation( this->shader.programId(), "uBillboardSize" );

    if (this->uColorLocation == static_cast<GLuint>(-1))
        std::fprintf(stderr, "Error: 'uColorLocation' not found");
    if (this->uProjCameraWorldLocation == static_cast<GLuint>(-1))
        std::fprintf(stderr, "Error: 'uProjCameraWorldLocation' not found");
    if (this->uParticleCenterWorldspaceLocation == static_cast<GLuint>(-1))
        std::fprintf(stderr, "Error: 'uParticleCenterWorldSpaceLocation' not found");
    if (this->uCameraRightWorldSpaceLocation == static_cast<GLuint>(-1))
        std::fprintf(stderr, "Error: 'uCameraRightWorldSpaceLocation' not found");
    if (this->uCameraUpWorldSpaceLocation == static_cast<GLuint>(-1))
        std::fprintf(stderr, "Error: 'uCameraUpWorldSpaceLocation' not found");
    if (this->uBillboardSizeLocation == static_cast<GLuint>(-1))
        std::fprintf(stderr, "Error: 'uBillboardSizeLocation' not found");

    float positions[] = {
        -0.5f,  0.5f, 0.0f,   // Top-left
        -0.5f, -0.5f, 0.0f,   // Bottom-left
        0.5f, -0.5f, 0.0f,   // Bottom-right

        0.5f,  0.5f, 0.0f,   // Top-right
        -0.5f,  0.5f, 0.0f,   // Top-left
        0.5f, -0.5f, 0.0f    // Bottom-right
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

void ParticleSystem::draw( Mat44f projCameraWorld, Mat44f viewMatrix ) {
    /*
     *  GL_ONE allows 'additive blending', which gives us the glow effect when
     *  sprites are stacked on each other
     */
    orderParticles( projCameraWorld );
    glEnable(GL_BLEND);
    glBlendFunc( GL_SRC_ALPHA, GL_ONE );

    GLuint currentProgram;
    glGetIntegerv(GL_CURRENT_PROGRAM, reinterpret_cast<GLint*>(&currentProgram));

    if (this->shader.programId() != currentProgram)
        glUseProgram( this->shader.programId() );   // Saves changing the shader if not needed

    glBindVertexArray(this->vao);
    glActiveTexture( GL_TEXTURE0 );
    glBindTexture(GL_TEXTURE_2D, this->textureId);

    // We need the camera's 'up' and 'right' vectors in world space
    // This is the same as the inverse of the view matrix
    // NOTE: The x and y are flipped in the viewMatrix because we are in ROW MAJOR
    //      NOTE2: Took me 12 hours to realise this...
    Vec3f cameraRightWorldSpace = normalize({ viewMatrix(0, 0), viewMatrix(0, 1), viewMatrix(0, 2) });
    Vec3f cameraUpWorldSpace    = normalize({ viewMatrix(1, 0), viewMatrix(1, 1), viewMatrix(1, 2) });

    glUniform3fv( this->uCameraRightWorldSpaceLocation, 1, &cameraRightWorldSpace.x );
    glUniform3fv( this->uCameraUpWorldSpaceLocation, 1, &cameraUpWorldSpace.x );

    // Doing one draw call for each particle is wasteful
    // Instead use instancing:
    //      https://www.opengl-tutorial.org/intermediate-tutorials/billboards-particles/particles-instancing/
    for (Particle particle : this->particles) {
        if (!particle.isDead()) {

            Mat44f model2worldParticle = make_translation(particle.position);
            Vec4f particleCenterWorldSpace = model2worldParticle * Vec4f{ 0.f, 0.f, 0.f, 1.f };

            glUniform3f(
                this->uParticleCenterWorldspaceLocation,
                particleCenterWorldSpace.x,
                particleCenterWorldSpace.y,
                particleCenterWorldSpace.z
            );

            glUniform2f( this->uBillboardSizeLocation, particle.size, particle.size );
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
