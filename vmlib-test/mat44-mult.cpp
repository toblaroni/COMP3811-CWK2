#include <catch2/catch_amalgamated.hpp>

#include "../vmlib/mat44.hpp"

using namespace Catch::Matchers;

TEST_CASE("4x4 matrix by matrix multiplication", "[mat44]") {
    SECTION("Simple 4x4 multiplication.") {
        Mat44f aLeft = {
            1.0f, 2.0f, 3.0f, 4.0f,
            5.0f, 6.0f, 7.0f, 8.0f,
            9.0f, 10.0f, 11.0f, 12.0f,
            13.0f, 14.0f, 15.0f, 16.0f
        };

        Mat44f aRight = {
            17.0f, 18.0f, 19.0f, 20.0f,
            21.0f, 22.0f, 23.0f, 24.0f,
            25.0f, 26.0f, 27.0f, 28.0f,
            29.0f, 30.0f, 31.0f, 32.0f
        };

        Mat44f expected = {
            250.0f, 260.0f, 270.0f, 280.0f,
            618.0f, 644.0f, 670.0f, 696.0f,
            986.0f, 1028.0f, 1070.0f, 1112.0f,
            1354.0f, 1412.0f, 1470.0f, 1528.0f
        };

        Mat44f result = aLeft * aRight;

        const float tolerance = 1e-5f; // Adjust tolerance as needed for precision
        for (int i = 0; i < 16; ++i) {
            REQUIRE_THAT(result.v[i], WithinAbs(expected.v[i], tolerance));
        }
    }
}

TEST_CASE("4x4 matrix by vector multiplication", "[mat44][vec4]") {
    SECTION("Simple Mat44f x Vec4f Test") {
        Mat44f matrix = {
            1.0f, 2.0f, 3.0f, 4.0f,
            5.0f, 6.0f, 7.0f, 8.0f,
            9.0f, 10.0f, 11.0f, 12.0f,
            13.0f, 14.0f, 15.0f, 16.0f
        };

        Vec4f vector = { 1.0f, 2.0f, 3.0f, 4.0f };

        Vec4f expected = {
            30.0f, 70.0f, 110.0f, 150.0f
        };

        const float tolerance = 1e-5f; // Adjust tolerance as needed for precision
        Vec4f result = matrix * vector;

        REQUIRE_THAT(result.x, WithinAbs(expected.x, tolerance));
        REQUIRE_THAT(result.y, WithinAbs(expected.y, tolerance));
        REQUIRE_THAT(result.z, WithinAbs(expected.z, tolerance));
        REQUIRE_THAT(result.w, WithinAbs(expected.w, tolerance));
    }
}
