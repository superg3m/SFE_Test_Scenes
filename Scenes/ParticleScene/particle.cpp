#include <Core/core.hpp>

struct Particle {
    Math::Vec3 position;
    Math::Vec3 velocity;
    // float lifetime = 0.0f;

    // float color;
    // Math::Quat orientation;
    // Math::Vec3 scale;
    // Geometry geometry;
    // Math::Vec3 acceleration;
};

const int MAX_PARTICLES = 1000;
Particle particles[MAX_PARTICLES];
u32 next_available_particle_index;

void update(float dt) {
    const float PARTICLE_SPAWN_COUNT_PER_FRAME = 5;
    for (int i = 0; i < PARTICLE_SPAWN_COUNT_PER_FRAME; i++) {
        Particle p;
        // make this have some randomness Random::GenerateF32(&seed);
        p.position = Math::Vec3(0, 0, 0);
        p.velocity = Math::Vec3(0, 1, 0);

        particles[next_available_particle_index] = p;
        next_available_particle_index = (next_available_particle_index + 1) % MAX_PARTICLES;
    }

    // update and render particles
    for (int i = 0; i < MAX_PARTICLES; i++) {
        Particle* p = &particles[i];
        p->position += p->velocity.scale(dt);
    }
}