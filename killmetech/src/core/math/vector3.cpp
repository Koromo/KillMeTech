#include "vector3.h"
#include "quaternion.h"
#include "math.h"
#include <cmath>
#include <cassert>

namespace killme
{
    const Vector3 Vector3::ZERO = {0, 0, 0};
    const Vector3 Vector3::UNIT_X = {1, 0, 0};
    const Vector3 Vector3::UNIT_Y = {0, 1, 0};
    const Vector3 Vector3::UNIT_Z = {0, 0, 1};

	Vector3::Vector3()
		: Vector3(Vector3::ZERO)
	{
	}

    Vector3::Vector3(float nx, float ny, float nz)
        : x(nx)
        , y(ny)
        , z(nz)
    {
    }

    const float& Vector3::operator [](size_t i) const
    {
        switch (i)
        {
        case 0: return x;
        case 1: return y;
        case 2: return z;
        default:
            assert(false && "Index out of range.");
            return x; // For warning
        }
    }

    float& Vector3::operator [](size_t i)
    {
        return const_cast<float&>(static_cast<const Vector3&>(*this)[i]);
    }

    bool operator ==(const Vector3& a, const Vector3& b)
    {
        return equalf(a.x, b.x) && equalf(a.y, b.y) && equalf(a.z, b.z);
    }

    bool operator !=(const Vector3& a, const Vector3& b)
    {
        return !(a == b);
    }

    const Vector3 operator +(const Vector3& v)
    {
        return v;
    }

    const Vector3 operator -(const Vector3& v)
    {
        return {-v.x, -v.y, -v.z};
    }

    const Vector3 operator +(const Vector3& a, const Vector3& b)
    {
        return {a.x + b.x, a.y + b.y, a.z + b.z};
    }

    const Vector3 operator -(const Vector3& a, const Vector3& b)
    {
        return {a.x - b.x, a.y - b.y, a.z - b.z};
    }

    const Vector3 operator *(const Vector3& v, float k)
    {
        return {v.x * k, v.y * k, v.z * k};
    }

    const Vector3 operator *(float k, const Vector3& v)
    {
        return v * k;
    }

    const Vector3 operator *(const Quaternion& q, const Vector3& v)
	{
        // See Ogre3D source
        const auto p = normalize(q);
        const Vector3 pv = {p.x, p.y, p.z};
        const auto uv = crossProduct(pv, v);
        const auto uuv = crossProduct(pv, uv);
        return v + uv * (2 * p.w) + uuv * 2;
    }

    const Vector3 operator /(const Vector3& v, float k)
    {
        assert(!equalf(k, 0) && "Division by zero error.");
        const auto invK = 1 / k;
        return v * invK;
    }

    Vector3& operator +=(Vector3& a, const Vector3& b)
    {
        a = a + b;
        return a;
    }

    Vector3& operator -=(Vector3& a, const Vector3& b)
    {
        a = a - b;
        return a;
    }

    Vector3& operator *=(Vector3& v, float k)
    {
        v = v * k;
        return v;
    }

    Vector3& operator /=(Vector3& v, float k)
    {
        assert(!equalf(k, 0) && "Division by zero error.");
        v = v / k;
        return v;
    }

    Vector3 scale(const Vector3& v, const Vector3& k)
    {
        return {v.x * k.x, v.y * k.y, v.z * k.z};
    }

    float norm(const Vector3& v)
    {
        return std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
    }

    Vector3 normalize(const Vector3& v)
    {
        assert(!equalf(norm(v), 0) && "Vector is zero.");
        return v / norm(v);
    }

    float dotProduct(const Vector3& a, const Vector3& b)
    {
        return a.x * b.x + a.y * b.y + a.z * b.z;
    }

    Vector3 crossProduct(const Vector3& a, const Vector3& b)
    {
        return {
            a.y * b.z - a.z * b.y,
            a.z * b.x - a.x * b.z,
            a.x * b.y - a.y * b.x
        };
    }
}