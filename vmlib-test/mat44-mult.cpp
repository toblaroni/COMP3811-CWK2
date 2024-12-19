#include <catch2/catch_amalgamated.hpp>

#include "../vmlib/mat44.hpp"

using namespace Catch::Matchers;

// Mat44 x Mat44 multiplication
TEST_CASE("4x4 matrix by 4x4 matrix multiplication", "[mat44]") {

    SECTION( "Mat44f-L x Mat44f-R == Known Result" ) {
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

        const float tolerance = 1e-6f; // Adjust tolerance as needed for precision
        for (int i = 0; i < 16; ++i) {
            REQUIRE_THAT(result.v[i], WithinAbs(expected.v[i], tolerance));
        }
    }
	
	/* This test will multiply an arbitrary matrix against its identity matrix
		to assess whether the result is the same as the original matrix. */
	SECTION( "Mat44f x Identity == Identity" )
	{
		Mat44f m = { {
			 1.f,  2.f,  3.f,  4.f,
			 5.f,  6.f,  7.f,  8.f,
			 9.f, 10.f, 11.f, 12.f,
			13.f, 14.f, 15.f, 16.f
		} };

		Mat44f result = m * kIdentity44f;

		const float tolerance = 1e-6f; // Adjust tolerance as needed for precision
		for (int i = 0; i < 16; ++i) {
            REQUIRE_THAT(result.v[i], WithinAbs(m.v[i], tolerance));
        }
	}

    SECTION( "Identity x Mat44f == Identity" )
	{
		Mat44f m = { {
			 1.f,  2.f,  3.f,  4.f,
			 5.f,  6.f,  7.f,  8.f,
			 9.f, 10.f, 11.f, 12.f,
			13.f, 14.f, 15.f, 16.f
		} };

		Mat44f result = kIdentity44f * m;

		const float tolerance = 1e-6f; // Adjust tolerance as needed for precision
		for (int i = 0; i < 16; ++i) {
            REQUIRE_THAT(result.v[i], WithinAbs(m.v[i], tolerance));
        }
	}

    SECTION( "Mat44f x 0 == 0" )
	{
		Mat44f m = { {
			 1.f,  2.f,  3.f,  4.f,
			 5.f,  6.f,  7.f,  8.f,
			 9.f, 10.f, 11.f, 12.f,
			13.f, 14.f, 15.f, 16.f
		} };

        Mat44f zero = { {
			 0.f,  0.f,  0.f,  0.f,
			 0.f,  0.f,  0.f,  0.f,
			 0.f,  0.f,  0.f,  0.f,
			 0.f,  0.f,  0.f,  0.f,
		} };

		Mat44f result = m * zero;

		const float tolerance = 1e-6f; // Adjust tolerance as needed for precision
		for (int i = 0; i < 16; ++i) {
            REQUIRE_THAT(result.v[i], WithinAbs(zero.v[i], tolerance));
        }
	}

    SECTION( "0 x Mat44f == 0" )
	{
		Mat44f m = { {
			 1.f,  2.f,  3.f,  4.f,
			 5.f,  6.f,  7.f,  8.f,
			 9.f, 10.f, 11.f, 12.f,
			13.f, 14.f, 15.f, 16.f
		} };

        Mat44f zero = { {
			 0.f,  0.f,  0.f,  0.f,
			 0.f,  0.f,  0.f,  0.f,
			 0.f,  0.f,  0.f,  0.f,
			 0.f,  0.f,  0.f,  0.f,
		} };

		Mat44f result = zero * m;

		const float tolerance = 1e-6f; // Adjust tolerance as needed for precision
		for (int i = 0; i < 16; ++i) {
            REQUIRE_THAT(result.v[i], WithinAbs(zero.v[i], tolerance));
        }
	}

    // Commutativity test
    SECTION( "Mat44f-L x Mat44f-R != Mat44f-R x Mat44f-L" )
	{
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

		Mat44f aLeftRight = aLeft * aRight;
        Mat44f aRightLeft = aRight * aLeft;

		const float tolerance = 1e-6f; // Adjust tolerance as needed for precision
		for (int i = 0; i < 16; ++i) {
            REQUIRE_THAT(aLeftRight.v[i], !WithinAbs(aRightLeft.v[i], tolerance));
        }
	}

    // Association test
    SECTION( "(Mat44f-1 x Mat44f-2) x Mat44f-3 == Mat44f-1 x (Mat44f-2 x Mat44f-3)" )
	{
        Mat44f aOne = {
            1.0f, 2.0f, 3.0f, 4.0f,
            5.0f, 6.0f, 7.0f, 8.0f,
            9.0f, 10.0f, 11.0f, 12.0f,
            13.0f, 14.0f, 15.0f, 16.0f
        };

        Mat44f aTwo = {
            17.0f, 18.0f, 19.0f, 20.0f,
            21.0f, 22.0f, 23.0f, 24.0f,
            25.0f, 26.0f, 27.0f, 28.0f,
            29.0f, 30.0f, 31.0f, 32.0f
        };

        Mat44f aThree = {
            33.0f, 34.0f, 35.0f, 36.0f,
            37.0f, 38.0f, 39.0f, 40.0f,
            41.0f, 42.0f, 43.0f, 44.0f,
            45.0f, 46.0f, 47.0f, 48.0f
        };

		Mat44f aOneTwo = aOne * aTwo;
        Mat44f aTwoThree = aTwo * aThree;

        Mat44f aOneTwo_Three = aOneTwo * aThree;
        Mat44f aOne_TwoThree = aOne * aTwoThree;

		const float tolerance = 1e-6f; // Adjust tolerance as needed for precision
		for (int i = 0; i < 16; ++i) {
            REQUIRE_THAT(aOneTwo_Three.v[i], WithinAbs(aOne_TwoThree.v[i], tolerance));
        }
	}

}

// Mat44 x Vec4 multiplication
TEST_CASE("4x4 matrix by 4x1 vector multiplication", "[mat44][vec4]") {

    SECTION("Simple Mat44f x Vec4f Test") {
        Mat44f matrix = {
            1.0f, 2.0f, 3.0f, 4.0f,
            5.0f, 6.0f, 7.0f, 8.0f,
            9.0f, 10.0f, 11.0f, 12.0f,
            13.0f, 14.0f, 15.0f, 16.0f
        };

        Vec4f vector = { 
			1.0f, 2.0f, 3.0f, 4.0f
		};

        Vec4f expected = {
            30.0f, 70.0f, 110.0f, 150.0f
        };

		Vec4f result = matrix * vector;

        const float tolerance = 1e-6f; // Adjust tolerance as needed for precision

        REQUIRE_THAT(result.x, WithinAbs(expected.x, tolerance));
        REQUIRE_THAT(result.y, WithinAbs(expected.y, tolerance));
        REQUIRE_THAT(result.z, WithinAbs(expected.z, tolerance));
        REQUIRE_THAT(result.w, WithinAbs(expected.w, tolerance));
    }

    /* This test will multiply the identity matrix against an arbitrary vector
	to assess whether the result is the same as the original matrix. */
	SECTION( "Identity x Vec4f == Vec4f" )
	{
        Vec4f vector = { 
			1.0f, 2.0f, 3.0f, 4.0f
		};

		Vec4f result = kIdentity44f * vector;

		const float tolerance = 1e-6f; // Adjust tolerance as needed for precision
        REQUIRE_THAT(result.x, WithinAbs(vector.x, tolerance));
        REQUIRE_THAT(result.y, WithinAbs(vector.y, tolerance));
        REQUIRE_THAT(result.z, WithinAbs(vector.z, tolerance));
        REQUIRE_THAT(result.w, WithinAbs(vector.w, tolerance));
	}

    SECTION( "0 x Vec4f == Vec4f(0)" )
	{
        Mat44f zero = { {
			 0.f,  0.f,  0.f,  0.f,
			 0.f,  0.f,  0.f,  0.f,
			 0.f,  0.f,  0.f,  0.f,
			 0.f,  0.f,  0.f,  0.f,
		} };

        Vec4f vector = { 
			1.0f, 2.0f, 3.0f, 4.0f
		};

		Vec4f result = zero * vector;

		const float tolerance = 1e-6f; // Adjust tolerance as needed for precision
        REQUIRE_THAT(result.x, WithinAbs(0.f, tolerance));
        REQUIRE_THAT(result.y, WithinAbs(0.f, tolerance));
        REQUIRE_THAT(result.z, WithinAbs(0.f, tolerance));
        REQUIRE_THAT(result.w, WithinAbs(0.f, tolerance));
	}
}
