#include <catch2/catch_amalgamated.hpp>

#include "../vmlib/mat44.hpp"

using namespace Catch::Matchers;

// Mat44 x Mat44 multiplication
TEST_CASE("4x4 matrix by Rotaion in X", "[mat44]") {

    SECTION( "Mat44f x make_rotation_x(0) == Mat44f" ) {
        Mat44f m = {
            1.0f, 2.0f, 3.0f, 4.0f,
            5.0f, 6.0f, 7.0f, 8.0f,
            9.0f, 10.0f, 11.0f, 12.0f,
            13.0f, 14.0f, 15.0f, 16.0f
        };

        Mat44f result = m * make_rotation_x(0);

        const float tolerance = 1e-6f; // Adjust tolerance as needed for precision
        for (int i = 0; i < 16; ++i) {
            REQUIRE_THAT(result.v[i], WithinAbs(m.v[i], tolerance));
        }
    }

    SECTION( "Mat44f x make_rotation_x(360) == Mat44f" ) {
        Mat44f m = {
            1.0f, 2.0f, 3.0f, 4.0f,
            5.0f, 6.0f, 7.0f, 8.0f,
            9.0f, 10.0f, 11.0f, 12.0f,
            13.0f, 14.0f, 15.0f, 16.0f
        };

        Mat44f result = m * make_rotation_x(2* std::numbers::pi_v<float>);

        const float tolerance = 1e-6f; // Adjust tolerance as needed for precision
        for (int i = 0; i < 16; ++i) {
            REQUIRE_THAT(result.v[i], WithinAbs(m.v[i], tolerance));
        }
    }
}

// Mat44 x Mat44 multiplication
TEST_CASE("4x4 matrix by Rotaion in Y", "[mat44]") {

    SECTION( "Mat44f x make_rotation_y(0) == Mat44f" ) {
        Mat44f m = {
            1.0f, 2.0f, 3.0f, 4.0f,
            5.0f, 6.0f, 7.0f, 8.0f,
            9.0f, 10.0f, 11.0f, 12.0f,
            13.0f, 14.0f, 15.0f, 16.0f
        };

        Mat44f result = m * make_rotation_y(0);

        const float tolerance = 1e-6f; // Adjust tolerance as needed for precision
        for (int i = 0; i < 16; ++i) {
            REQUIRE_THAT(result.v[i], WithinAbs(m.v[i], tolerance));
        }
    }

    SECTION( "Mat44f x make_rotation_y(360) == Mat44f" ) {
        Mat44f m = {
            1.0f, 2.0f, 3.0f, 4.0f,
            5.0f, 6.0f, 7.0f, 8.0f,
            9.0f, 10.0f, 11.0f, 12.0f,
            13.0f, 14.0f, 15.0f, 16.0f
        };

        Mat44f result = m * make_rotation_y(2* std::numbers::pi_v<float>);

        const float tolerance = 1e-6f; // Adjust tolerance as needed for precision
        for (int i = 0; i < 16; ++i) {
            REQUIRE_THAT(result.v[i], WithinAbs(m.v[i], tolerance));
        }
    }
}
