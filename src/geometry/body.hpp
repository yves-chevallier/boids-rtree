/**
 * Rigid body definition
 */
#pragma once

#include "geometry/angle.hpp"
#include "geometry/vector2.hpp"

class Body {
public:
    using point_2d = Vector2f;

    Body(point_2d position = {0, 0}, point_2d velocity = {0, 0},
         float mass = 1.0f)
        : position(position), velocity{velocity}, mass(mass) {}

    point_2d position;
    point_2d velocity;

    float angle() const { return std::atan2(velocity.y, velocity.x); }
    float speed() const { return std::hypot(velocity.x, velocity.y); }
    float inertia() const { return mass * speed(); }
    float kineticEnergy() const { return 0.5f * mass * speed() * speed(); }

    void applyForce(point_2d force, float dt = 1.0f) {
        velocity += force / mass * dt;
    }

    struct ByPos {
        using result_type = point_2d;
        result_type const& operator()(Body const& body) const { return body.position; }
    };

    float mass;
};