#include <catch2/catch_amalgamated.hpp>

#include <numbers>

#include "../vmlib/mat44.hpp"

using namespace Catch::Matchers;


TEST_CASE( "Perspective projection", "[mat44]" )
{
	static constexpr float tolerance = 1e-5f;

	// "Standard" projection matrix presented in the exercises. Assumes
	// standard window size (e.g., 1280x720).
	//
	// Field of view (FOV) = 60 degrees
	// Window size is 1280x720 and we defined the aspect ratio as w/h
	// Near plane at 0.1 and far at 100
	SECTION( "Standard, from the exercises" )
	{
		Mat44f proj = make_perspective_projection(
			60.f * std::numbers::pi_v<float> / 180.f,
			1280/float(720),
			0.1f, 100.f
		);

        Mat44f expected = {
            0.974279f, 0.f,       0.f,        0.f,
            0.f,       1.732051f, 0.f,        0.f,
            0.f,       0.f,      -1.002002f, -0.200200f,
            0.f,       0.f,      -1.f,        0.f
        };


        for (int i = 0; i < 16; ++i) {
            REQUIRE_THAT(proj.v[i], WithinAbs(expected.v[i], tolerance));
        }
	}

    SECTION( "Split-screen" )
	{
		Mat44f proj = make_perspective_projection(
			60.f * std::numbers::pi_v<float> / 180.f,
			640/float(720),
			0.1f, 100.f
		);

        Mat44f expected = {
            1.948557f, 0.f,       0.f,        0.f,
            0.f,       1.732051f, 0.f,        0.f,
            0.f,       0.f,      -1.002002f, -0.200200f,
            0.f,       0.f,      -1.f,        0.f
        };


        for (int i = 0; i < 16; ++i) {
            REQUIRE_THAT(proj.v[i], WithinAbs(expected.v[i], tolerance));
        }
	}

    SECTION( "Wide FOV on Ultrawide screen" )
	{
		Mat44f proj = make_perspective_projection(
			90.f * std::numbers::pi_v<float> / 180.f,
            3440/float(1440),
			0.1f, 100.f
		);

        Mat44f expected = {
            0.4186f, 0.f,       0.f,        0.f,
            0.f,     1.0f,      0.f,        0.f,
            0.f,     0.f,      -1.002002f, -0.200200f,
            0.f,     0.f,      -1.f,        0.f
        };


        for (int i = 0; i < 16; ++i) {
            REQUIRE_THAT(proj.v[i], WithinAbs(expected.v[i], tolerance));
        }
	}

    SECTION( "Altered near and far plane" )
	{
		Mat44f proj = make_perspective_projection(
			60.f * std::numbers::pi_v<float> / 180.f,
			1280/float(720),
			1.f, 10.f
		);

        Mat44f expected = {
            0.974278f, 0.f,       0.f,        0.f,
            0.f,       1.732051f, 0.f,        0.f,
            0.f,       0.f,      -1.222222f, -2.222222f,
            0.f,       0.f,      -1.f,        0.f
        };


        for (int i = 0; i < 16; ++i) {
            REQUIRE_THAT(proj.v[i], WithinAbs(expected.v[i], tolerance));
        }
	}
}
