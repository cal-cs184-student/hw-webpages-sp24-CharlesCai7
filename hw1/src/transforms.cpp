#include "transforms.h"

#include "CGL/matrix3x3.h"
#include "CGL/vector2D.h"
#include "CGL/vector3D.h"

namespace CGL {

Vector2D operator*(const Matrix3x3 &m, const Vector2D &v) {
	Vector3D mv = m * Vector3D(v.x, v.y, 1);
	return Vector2D(mv.x / mv.z, mv.y / mv.z);
}

Matrix3x3 translate(float dx, float dy) {
	// Part 3: Fill this in.
	Matrix3x3 m;
	m(0, 2) = dx;
	m(1, 2) = dy;
	return m;
}

Matrix3x3 scale(float sx, float sy) {
	// Part 3: Fill this in.
	Matrix3x3 m;
	m(0, 0) = sx;
	m(1, 1) = sy;
	return m;
}

// The input argument is in degrees counterclockwise
Matrix3x3 rotate(float deg) {
	// Part 3: Fill this in.
	Matrix3x3 m;
	float rad = deg * PI / 180;
	m(0, 0) = cos(rad);
	m(0, 1) = -sin(rad);
	m(1, 0) = sin(rad);
	m(1, 1) = cos(rad);
	return m;
}

}
