#ifndef MAT44_HPP_E7187A26_469E_48AD_A3D2_63150F05A4CA
#define MAT44_HPP_E7187A26_469E_48AD_A3D2_63150F05A4CA
// SOLUTION_TAGS: gl-(ex-[^12]|cw-2)

#include <cmath>
#include <cassert>
#include <cstdlib>

#include "vec3.hpp"
#include "vec4.hpp"

/** Mat44f: 4x4 matrix with floats
 *
 * See vec2f.hpp for discussion. Similar to the implementation, the Mat44f is
 * intentionally kept simple and somewhat bare bones.
 *
 * The matrix is stored in row-major order (careful when passing it to OpenGL).
 *
 * The overloaded operator () allows access to individual elements. Example:
 *    Mat44f m = ...;
 *    float m12 = m(1,2);
 *    m(0,3) = 3.f;
 *
 * The matrix is arranged as:
 *
 *   ⎛ 0,0  0,1  0,2  0,3 ⎞
 *   ⎜ 1,0  1,1  1,2  1,3 ⎟
 *   ⎜ 2,0  2,1  2,2  2,3 ⎟
 *   ⎝ 3,0  3,1  3,2  3,3 ⎠
 */
struct Mat44f
{
	float v[16];

	constexpr
	float& operator() (std::size_t aI, std::size_t aJ) noexcept
	{
		assert( aI < 4 && aJ < 4 );
		return v[aI*4 + aJ];
	}
	constexpr
	float const& operator() (std::size_t aI, std::size_t aJ) const noexcept
	{
		assert( aI < 4 && aJ < 4 );
		return v[aI*4 + aJ];
	}
};

// Identity matrix
constexpr Mat44f kIdentity44f = { {
	1.f, 0.f, 0.f, 0.f,
	0.f, 1.f, 0.f, 0.f,
	0.f, 0.f, 1.f, 0.f,
	0.f, 0.f, 0.f, 1.f
} };

// Common operators for Mat44f.
// Note that you will need to implement these yourself.

constexpr 
Mat44f operator*(Mat44f const &aLeft, Mat44f const &aRight) noexcept {
    Mat44f ret = {};

	for (int row = 0; row < 4; ++row) {
		for (int col = 0; col < 4; ++col) {
			ret(row, col) = 
				aLeft(row, 0) * aRight(0, col) +
				aLeft(row, 1) * aRight(1, col) +
				aLeft(row, 2) * aRight(2, col) +
				aLeft(row, 3) * aRight(3, col);
		}
	}

	return {
		ret.v[0], ret.v[1], ret.v[2], ret.v[3], 
		ret.v[4], ret.v[5], ret.v[6], ret.v[7], 
		ret.v[8], ret.v[9], ret.v[10], ret.v[11], 
		ret.v[12], ret.v[13], ret.v[14], ret.v[15], 
	};
}

constexpr
Vec4f operator*( Mat44f const& aLeft, Vec4f const& aRight ) noexcept
{
	float x = aLeft.v[0]*aRight.x  + aLeft.v[1]*aRight.y  + aLeft.v[2]*aRight.z  + aLeft.v[3]*aRight.w;
	float y = aLeft.v[4]*aRight.x  + aLeft.v[5]*aRight.y  + aLeft.v[6]*aRight.z  + aLeft.v[7]*aRight.w;
	float z = aLeft.v[8]*aRight.x  + aLeft.v[9]*aRight.y  + aLeft.v[10]*aRight.z + aLeft.v[11]*aRight.w;
	float w = aLeft.v[12]*aRight.x + aLeft.v[13]*aRight.y + aLeft.v[14]*aRight.z + aLeft.v[15]*aRight.w;

	return { x, y, z, w };
}

// Functions:


inline
Mat44f make_rotation_x( float aAngle ) noexcept
{
	float cos_a = std::cos( aAngle );
	float sin_a = std::sin( aAngle );

	return {
		1.f, 0.f, 0.f, 0.f,
		0.f, cos_a, -sin_a, 0.f,
		0.f, sin_a, cos_a, 0.f,
		0.f, 0.f, 0.f, 1.f,
	};
}


inline
Mat44f make_rotation_y( float aAngle ) noexcept
{
	float cos_a = std::cos( aAngle );
	float sin_a = std::sin( aAngle );

	return {
		cos_a, 0.f, sin_a, 0.f,
		0.f, 1.f, 0.f, 0.f,
		-sin_a, 0.f, cos_a, 0.f,
		0.f, 0.f, 0.f, 1.f,
	};
}

inline
Mat44f make_rotation_z( float aAngle ) noexcept
{
	float cos_a = std::cos( aAngle );
	float sin_a = std::sin( aAngle );

	return {
		cos_a, -sin_a, 0.f, 0.f,
		sin_a, cos_a, 0.f, 0.f,
		0.f, 0.f, 1.f, 0.f,
		0.f, 0.f, 0.f, 1.f,
	};
}

inline
Mat44f make_translation( Vec3f aTranslation ) noexcept
{
	return {
		1.f, 0.f, 0.f, aTranslation.x,
		0.f, 1.f, 0.f, aTranslation.y,
		0.f, 0.f, 1.f, aTranslation.z,
		0.f, 0.f, 0.f, 1.f,
	};
}

inline
Mat44f make_perspective_projection( float aFovInRadians, float aAspect, float aNear, float aFar ) noexcept
{
	float s = 1.f / std::tan(aFovInRadians / 2.f);
	float sx = s / aAspect;

	float a = -(aFar + aNear) / (aFar - aNear);
	float b = -2.f * (aFar * aNear) / (aFar - aNear);

	return {
		sx, 0.f, 0.f, 0.f,
		0.f, s, 0.f, 0.f,
		0.f, 0.f, a, b,
		0.f, 0.f, -1.f, 0.f
	};
}

#endif // MAT44_HPP_E7187A26_469E_48AD_A3D2_63150F05A4CA
