#ifndef PARTICLE_HPP
#define PARTICLE_HPP

#include "../vmlib/vec3.hpp"
#include "../vmlib/vec4.hpp"

/*
 *  === Particles ===
 *  https://learnopengl.com/index.php?p=In-Practice/2D-Game/Particles
 */
struct Particle {
    Vec3f velocity = { 0.f, 0.f, 0.f };
    Vec3f position = { 0.f, 0.f, 0.f };
    Vec4f color    = { 1.f, 1.f, 1.f, 1.f };
    float lifetime = 0.f;
};

#endif