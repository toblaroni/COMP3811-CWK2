#include "vehicle.hpp"


SimpleMeshData make_vehicle() {
    Material spaceshipRed = {
        {0.5f, 0.0f, 0.0f },
        {0.9f, 0.1f, 0.1f},
        {0.8f, 0.3f, 0.3f},
        50.0f,             
        {0.2f, 0.0f, 0.0f},
        1.0f               
    };

    Material spaceshipGrey = {
        {0.2f, 0.2f, 0.2f},
        {0.4f, 0.4f, 0.4f},
        {0.1f, 0.1f, 0.1f},
        5.0f,              
        {0.0f, 0.0f, 0.0f},
        1.0f               
    };

    Material spaceshipLightGrey = {
        {0.4f, 0.4f, 0.4f},
        {0.4f, 0.4f, 0.4f},
        {0.3f, 0.3f, 0.3f},
        50.0f,              
        {0.0f, 0.0f, 0.0f},
        1.0f              
    };

    Material lightbulbMaterial = {
        {0.2f, 0.2f, 0.2f},
        {0.8f, 0.8f, 0.0f},
        {1.0f, 1.0f, 1.0f},
        32.0f,             
        {1.0f, 1.0f, 0.0f},
        1.0f               
    };

    auto top = make_cone(
        true, 16, spaceshipRed,
        make_translation({0.f, 0.5f, 0.f}) *
        make_scaling(.1f, .2f, .1f) *
        make_rotation_z(0.5f * std::numbers::pi_v<float>)
    );

    auto middle = make_cylinder(
        true, 16, spaceshipLightGrey,
        make_translation({0.f, 0.2f, 0.f}) * 
        make_scaling(.1f, .3f, .1f) * 
        make_rotation_z(0.5f * std::numbers::pi_v<float>)
    );

    auto booster1 = make_cone(
        false, 16, spaceshipGrey,
        make_translation({0.04f, 0.17f, 0.04f}) * 
        make_scaling(.04f, .06f, .04f) * 
        make_rotation_z(0.5f * std::numbers::pi_v<float>)
    );

    auto booster2 = make_cone(
        false, 16, spaceshipGrey,
        make_translation({0.04f, 0.17f, -0.04f}) * 
        make_scaling(.04f, .06f, .04f) * 
        make_rotation_z(0.5f * std::numbers::pi_v<float>)
    );

    auto booster3 = make_cone(
        false, 16, spaceshipGrey,
        make_translation({-0.04f, 0.17f, 0.04f}) * 
        make_scaling(.04f, .06f, .04f) * 
        make_rotation_z(0.5f * std::numbers::pi_v<float>)
    );

    auto booster4 = make_cone(
        false, 16, spaceshipGrey,
        make_translation({-0.04f, 0.17f, -0.04f}) *
        make_scaling(.04f, .06f, .04f) * 
        make_rotation_z(0.5f * std::numbers::pi_v<float>)
    );

    // RED
    auto leg1 = make_cylinder(
        true, 16, spaceshipRed,
        make_translation({0.15f, 0.05f, 0.15f}) *
        make_rotation_y(-0.75f * std::numbers::pi_v<float>) *
        make_rotation_x(0.15f * std::numbers::pi_v<float>) *
        make_shearing(0.f, 0.f, 0.f, 1.f, 0.f, 0.f) *
        make_scaling(.01f, .25f, .05f) * make_rotation_z(0.5f * std::numbers::pi_v<float>)
    );

    // RED
    auto leg2 = make_cylinder(
        true, 16, spaceshipRed,
        make_translation({-0.15f, 0.05f, -0.15f}) *
        make_rotation_y(0.25f * std::numbers::pi_v<float>) *
        make_rotation_x(0.15f * std::numbers::pi_v<float>) *
        make_shearing(0.f, 0.f, 0.f, 1.f, 0.f, 0.f) *
        make_scaling(.01f, .25f, .05f) * make_rotation_z(0.5f * std::numbers::pi_v<float>)
    );

    // RED
    auto leg3 = make_cylinder(
        true, 16, spaceshipRed,
        make_translation({-0.15f, 0.05f, 0.15f}) *
        make_rotation_y(0.75f * std::numbers::pi_v<float>) *
        make_rotation_x(0.15f * std::numbers::pi_v<float>) *
        make_shearing(0.f, 0.f, 0.f, 1.f, 0.f, 0.f) *
        make_scaling(.01f, .25f, .05f) * make_rotation_z(0.5f * std::numbers::pi_v<float>)
    );

    auto leg4 = make_cylinder(
        true, 16, spaceshipRed,
        make_translation({0.15f, 0.05f, -0.15f}) *
        make_rotation_y(-0.25f * std::numbers::pi_v<float>) *
        make_rotation_x(0.15f * std::numbers::pi_v<float>) *
        make_shearing(0.f, 0.f, 0.f, 1.f, 0.f, 0.f) *
        make_scaling(.01f, .25f, .05f) * make_rotation_z(0.5f * std::numbers::pi_v<float>)
    );

    auto point = make_cube(
        lightbulbMaterial,
        make_translation({0.f, 0.7f, 0.f}) * make_scaling(.025f, .025f, .025f)
    );

    auto upper = concatenate(point, top);

    auto shaft = concatenate(upper, middle);

    auto b12 = concatenate(booster1, booster2);
    auto b34 = concatenate(booster3, booster4);
    auto boosters = concatenate(b12, b34);

    auto l12 = concatenate(leg1, leg2);
    auto l34 = concatenate(leg3, leg4);
    auto legs = concatenate(l12, l34);

    auto body = concatenate(shaft, legs);

    return concatenate(body, boosters);
}