#ifndef PARTICLE_HPP
#define PARTICLE_HPP

#include "../vmlib/vec3.hpp"
#include "../vmlib/vec4.hpp"
#include "../vmlib/mat44.hpp"
#include "../support/program.hpp"

#include <GLFW/glfw3.h>

/*
 *  === Particles ===
 *  https://learnopengl.com/index.php?p=In-Practice/2D-Game/Particles
 */

struct Particle {
    Vec3f velocity = { 0.f, 0.f, 0.f };
    Vec3f position = { 0.f, 0.f, 0.f };
    Vec4f color    = { 1.f, 1.f, 1.f, 1.f };
    float lifetime = 0.f;

    bool isDead() { return lifetime <= 0.f; }
};

class ParticleSystem {
public:
    ParticleSystem( ShaderProgram& shader, GLuint textureId, unsigned int amount );

    // Add offset?
    void update( float dt, Vec3f objPosition, Vec3f objVelocity, unsigned int newParticles );
    void draw( GLuint, Mat44f );
private:
    ShaderProgram &shader;
    std::vector<Particle> particles;
    unsigned int lastUsedParticle;
    unsigned int numParticles;
    GLuint vao;
    GLuint textureId;

    // Uniform locations
    GLuint uOffsetLocation; 
    GLuint uColorLocation;

    void init();    // Initialises vao
    unsigned int firstUnusedParticle();
    void respawnParticle( Particle& particle, Vec3f objPosition, Vec3f objVelocity );
};

#endif  // PARTICLE_HPP