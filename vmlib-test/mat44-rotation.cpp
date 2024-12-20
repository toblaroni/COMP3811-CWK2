#include <catch2/catch_amalgamated.hpp>

#include <numbers>

#include "../vmlib/mat44.hpp"

using namespace Catch::Matchers;


// Mat44 x Rotation in X
TEST_CASE("4x4 matrix by Rotaion in X", "[rotation][mat44]") {

    static constexpr float tolerance = 1e-5f;

    SECTION( "make_rotation_x(0) == Identity" ) {

        Mat44f result = make_rotation_x(0.f);

        for (int i = 0; i < 16; ++i) {
            REQUIRE_THAT(result.v[i], WithinAbs(kIdentity44f.v[i], tolerance));
        }
    }

    SECTION( "make_rotation_x(360) == Identity" ) {

        Mat44f result = make_rotation_x(2* std::numbers::pi_v<float>);

        for (int i = 0; i < 16; ++i) {
            REQUIRE_THAT(result.v[i], WithinAbs(kIdentity44f.v[i], tolerance));
        }
    }

    SECTION( "make_rotation_x(theta) x make_rotation_x(-theta) == Identity" ) {

        float arbitrary_angle = 238.f * std::numbers::pi_v<float> / 180.f;

        Mat44f result = make_rotation_x(arbitrary_angle) * make_rotation_x(-arbitrary_angle);

        for (int i = 0; i < 16; ++i) {
            REQUIRE_THAT(result.v[i], WithinAbs(kIdentity44f.v[i], tolerance));
        }
    }

    SECTION( "make_rotation_x(90) == Known result" ) {

        Mat44f expected = {
            1.0f,  0.0f,  0.0f, 0.0f,
            0.0f,  0.0f, -1.0f, 0.0f,
            0.0f,  1.0f,  0.0f, 0.0f,
            0.0f,  0.0f,  0.0f, 1.0f
        };

        Mat44f result = make_rotation_x(std::numbers::pi_v<float>/2.f);

        for (int i = 0; i < 16; ++i) {
            REQUIRE_THAT(result.v[i], WithinAbs(expected.v[i], tolerance));
        }
    }

    SECTION( "make_rotation_x(-90) == Known result" ) {

        Mat44f expected = {
            1.0f,  0.0f,  0.0f, 0.0f,
            0.0f,  0.0f,  1.0f, 0.0f,
            0.0f, -1.0f,  0.0f, 0.0f,
            0.0f,  0.0f,  0.0f, 1.0f
        };

        Mat44f result = make_rotation_x(-std::numbers::pi_v<float> / 2.f);

        for (int i = 0; i < 16; ++i) {
            REQUIRE_THAT(result.v[i], WithinAbs(expected.v[i], tolerance));
        }
    }

}

// Mat44 x Rotation in Y
TEST_CASE("4x4 matrix by Rotaion in Y", "[rotation][mat44]") {

    static constexpr float tolerance = 1e-5f;

    SECTION( "make_rotation_y(0) == Identity" ) {

        Mat44f result = make_rotation_y(0);

        for (int i = 0; i < 16; ++i) {
            REQUIRE_THAT(result.v[i], WithinAbs(kIdentity44f.v[i], tolerance));
        }
    }

    SECTION( "make_rotation_y(360) == Identity" ) {

        Mat44f result = make_rotation_y(2* std::numbers::pi_v<float>);

        for (int i = 0; i < 16; ++i) {
            REQUIRE_THAT(result.v[i], WithinAbs(kIdentity44f.v[i], tolerance));
        }
    }

    SECTION( "make_rotation_y(theta) x make_rotation_y(-theta) == Identity" ) {

        float arbitrary_angle = 17.f * std::numbers::pi_v<float> / 180.f;

        Mat44f result = make_rotation_y(arbitrary_angle) * make_rotation_y(-arbitrary_angle);

        for (int i = 0; i < 16; ++i) {
            REQUIRE_THAT(result.v[i], WithinAbs(kIdentity44f.v[i], tolerance));
        }
    }

    SECTION( "make_rotation_y(90) == Known result " ) {

        Mat44f expected = {
            0.0f, 0.0f,  1.0f, 0.0f,
            0.0f, 1.0f,  0.0f, 0.0f,
           -1.0f, 0.0f,  0.0f, 0.0f,
            0.0f, 0.0f,  0.0f, 1.0f
        };

        Mat44f result = make_rotation_y(std::numbers::pi_v<float> / 2.f);

        for (int i = 0; i < 16; ++i) {
            REQUIRE_THAT(result.v[i], WithinAbs(expected.v[i], tolerance));
        }
    }

    SECTION( "make_rotation_y(-90) == Known result" ) {

        Mat44f expected = {
            0.0f,  0.0f, -1.0f, 0.0f,
            0.0f,  1.0f,  0.0f, 0.0f,
            1.0f,  0.0f,  0.0f, 0.0f,
            0.0f,  0.0f,  0.0f, 1.0f
        };

        Mat44f result = make_rotation_y(-std::numbers::pi_v<float> / 2.f);

        for (int i = 0; i < 16; ++i) {
            REQUIRE_THAT(result.v[i], WithinAbs(expected.v[i], tolerance));
        }
    }

}

// Mat44 x Rotation in Z
TEST_CASE("4x4 matrix by Rotaion in Z", "[rotation][mat44]") {

    static constexpr float tolerance = 1e-5f;

    SECTION( "make_rotation_z(0) == Identity" ) {

        Mat44f result = make_rotation_z(0);

        for (int i = 0; i < 16; ++i) {
            REQUIRE_THAT(result.v[i], WithinAbs(kIdentity44f.v[i], tolerance));
        }
    }

    SECTION( "make_rotation_z(360) == Identity" ) {

        Mat44f result = make_rotation_z(2* std::numbers::pi_v<float>);

        for (int i = 0; i < 16; ++i) {
            REQUIRE_THAT(result.v[i], WithinAbs(kIdentity44f.v[i], tolerance));
        }
    }

    SECTION( "make_rotation_z(theta) x make_rotation_z(-theta) == Identity" ) {

        float arbitrary_angle = 967.f * std::numbers::pi_v<float> / 180.f;

        Mat44f result = make_rotation_z(arbitrary_angle) * make_rotation_z(-arbitrary_angle);

        for (int i = 0; i < 16; ++i) {
            REQUIRE_THAT(result.v[i], WithinAbs(kIdentity44f.v[i], tolerance));
        }
    }

    SECTION( "make_rotation_z(90) == Known result" ) {

        Mat44f expected = {
            0.0f, -1.0f, 0.0f, 0.0f,
            1.0f,  0.0f, 0.0f, 0.0f,
            0.0f,  0.0f, 1.0f, 0.0f,
            0.0f,  0.0f, 0.0f, 1.0f
        };

        Mat44f result = make_rotation_z(std::numbers::pi_v<float> / 2.f);

        for (int i = 0; i < 16; ++i) {
            REQUIRE_THAT(result.v[i], WithinAbs(expected.v[i], tolerance));
        }
    }

    SECTION( "make_rotation_z(-90) == Known result" ) {

        Mat44f expected = {
            0.0f,  1.0f,  0.0f, 0.0f,
           -1.0f,  0.0f,  0.0f, 0.0f,
            0.0f,  0.0f,  1.0f, 0.0f,
            0.0f,  0.0f,  0.0f, 1.0f
        };

        Mat44f result = make_rotation_z(-std::numbers::pi_v<float> / 2.f);

        for (int i = 0; i < 16; ++i) {
            REQUIRE_THAT(result.v[i], WithinAbs(expected.v[i], tolerance));
        }
    }

}
