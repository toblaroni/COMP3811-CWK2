#ifndef VEHICLE_HPP
#define VEHICLE_HPP

#include <numbers>

#include "simple_mesh.hpp"
#include "../vmlib/mat44.hpp"
#include "shapes/cylinder.hpp"
#include "shapes/cone.hpp"
#include "shapes/cube.hpp"


SimpleMeshData make_vehicle();

#endif // VEHICLE_HPP