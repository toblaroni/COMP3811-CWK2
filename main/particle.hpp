#ifndef PARTICLE_HPP
#define PARTICLE_HPP

#include "../vmlib/vec3.hpp"
#include "../vmlib/vec4.hpp"
#include "../vmlib/mat44.hpp"
#include "../support/program.hpp"

#include <algorithm>
/*
 *  === Particles ===
 *  https://learnopengl.com/index.php?p=In-Practice/2D-Game/Particles
 */

struct Particle {
    Vec3f velocity = { 0.f, 0.f, 0.f };
    Vec3f position = { 0.f, 0.f, 0.f };
    Vec4f color    = { 1.f, 1.f, 1.f, 1.f };
    float lifetime = 0.f;
    float initialSize = 0.5f;
    float size     = 0.5f;
    float ndcDepth;       // This is the squared distance to the camera.

    bool isDead() { return lifetime <= 0.f; }

    bool operator<(const Particle& that) const {
        return this->ndcDepth > that.ndcDepth;
    }
};

class ParticleSystem {
public:
    ParticleSystem( ShaderProgram& shader, GLuint textureId, unsigned int amount );

    // Add offset?
    void update( float dt, Vec3f objPosition, Vec3f objVelocity, unsigned int newParticles, Vec3f cameraPos );
    void draw( Mat44f, Mat44f );
    void reset( Vec3f );
private:
    const ShaderProgram &shader;
    const GLuint textureId;
    unsigned int numParticles;
    unsigned int lastUsedParticle;
    std::vector<Particle> particles;
    GLuint vao;

    // Uniform locations
    GLuint uColorLocation;
    GLuint uProjCameraWorldLocation;

    // Billboarding
    // https://www.opengl-tutorial.org/intermediate-tutorials/billboards-particles/billboards/
    GLuint uParticleCenterWorldspaceLocation;
    GLuint uCameraRightWorldSpaceLocation;
    GLuint uCameraUpWorldSpaceLocation;
    GLuint uBillboardSizeLocation;

    void init();    // Initialises vao
    unsigned int firstUnusedParticle();
    void respawnParticle( Particle& particle, Vec3f objPosition, Vec3f objVelocity );
    void sortParticles( ) {
        std::sort(this->particles.begin(), this->particles.end());
    };
    void orderParticles( Mat44f );
};

#endif  // PARTICLE_HPP
