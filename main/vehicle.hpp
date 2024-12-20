#ifndef VEHICLE_HPP
#define VEHICLE_HPP

#include <numbers>

#include "simple_mesh.hpp"
#include "../vmlib/mat44.hpp"
#include "shapes/cylinder.hpp"
#include "shapes/cone.hpp"
#include "shapes/cube.hpp"

struct VehicleCtrl_ {
    bool hasLaunched = false;       // This lets us know if the ship has launched already (for particles)
    bool launch = false;
    Vec3f origin = { 3.f, 0.f, -5.f };
    Vec3f position = origin;

    float time = 0.f;
    float theta = 0.f;

    // This is needed for particles
    Vec3f velocity = { 0.f, 0.f, 0.f };

    void resetVehicle() {
        hasLaunched = false;
        launch = false;
        position = origin;
        time = 0.f;
        theta = 0.f;
    }
};

SimpleMeshData make_vehicle();

#endif // VEHICLE_HPP