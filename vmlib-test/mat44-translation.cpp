#include <catch2/catch_amalgamated.hpp>

#include "../vmlib/mat44.hpp"

using namespace Catch::Matchers;


TEST_CASE("4x4 matrix by Translation", "[translation][mat44]") {

    static constexpr float tolerance = 1e-5f;

    SECTION( "make_translation(0) == Identity" ) {

        Mat44f result = make_translation({0.f, 0.f, 0.f});

        for (int i = 0; i < 16; ++i) {
            REQUIRE_THAT(result.v[i], WithinAbs(kIdentity44f.v[i], tolerance));
        }
    }

    SECTION( "make_translation(x) x make_translation(-x) == Identity" ) {

        Vec3f arbitrary_translation = {20.f, 45.73f, -9872.f};

        Mat44f result = make_translation(arbitrary_translation) * make_translation(-arbitrary_translation);

        for (int i = 0; i < 16; ++i) {
            REQUIRE_THAT(result.v[i], WithinAbs(kIdentity44f.v[i], tolerance));
        }
    }

    SECTION( "make_translation(1, 2, 3) == Known result" ) {

        Mat44f expected = {
            1.f, 0.f, 0.f, 1.f,
            0.f, 1.f, 0.f, 2.f,
            0.f, 0.f, 1.f, 3.f,
            0.f, 0.f, 0.f, 1.f,
        };

        Mat44f result = make_translation({1.f, 2.f, 3.f});

        for (int i = 0; i < 16; ++i) {
            REQUIRE_THAT(result.v[i], WithinAbs(expected.v[i], tolerance));
        }
    }

    SECTION( "make_translation(-3, -2, -1) == Known result" ) {

        Mat44f expected = {
            1.f, 0.f, 0.f, -3.f,
            0.f, 1.f, 0.f, -2.f,
            0.f, 0.f, 1.f, -1.f,
            0.f, 0.f, 0.f, 1.f,
        };

        Mat44f result = make_translation({-3.f, -2.f, -1.f});

        for (int i = 0; i < 16; ++i) {
            REQUIRE_THAT(result.v[i], WithinAbs(expected.v[i], tolerance));
        }
    }

}


