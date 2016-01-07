#ifndef _KILLME_VECTOR3_H_
#define _KILLME_VECTOR3_H_

namespace killme
{
    class Quaternion;

    /** 3D vector */
    class Vector3
    {
    public:
        /** Elements */
        float x, y, z;

        /** Construct zero vector */
        Vector3();

        /** Construct with a initialize value */
        Vector3(float nx, float ny, float nz);

        /** Accessor */
        const float& operator[](size_t i) const;
        float& operator[](size_t i);

        /** Specific vector definitions */
        static const Vector3 ZERO;
        static const Vector3 UNIT_X;
        static const Vector3 UNIT_Y;
        static const Vector3 UNIT_Z;
    };

    /** Operator overloads */
    bool operator==(const Vector3& a, const Vector3& b);
    bool operator!=(const Vector3& a, const Vector3& b);

    const Vector3 operator+(const Vector3& v);
    const Vector3 operator-(const Vector3& v);

    const Vector3 operator+(const Vector3& a, const Vector3& b);
    const Vector3 operator-(const Vector3& a, const Vector3& b);
    const Vector3 operator*(const Vector3& v, float k);
    const Vector3 operator*(float k, const Vector3& v);
    const Vector3 operator*(const Quaternion& q, const Vector3& v);
    const Vector3 operator/(const Vector3& v, float k);

    Vector3& operator+=(Vector3& a, const Vector3& b);
    Vector3& operator-=(Vector3& a, const Vector3& b);
    Vector3& operator*=(Vector3& v, float k);
    Vector3& operator/=(Vector3& v, float k);

    /** Scale vector */
    Vector3 scale(const Vector3& v, const Vector3& k);

    /** Returns norm of vector */
    float norm(const Vector3& v);

    /** Returns normalized vector */
    Vector3 normalize(const Vector3& v);

    /** Returns dot product value */
    float dotProduct(const Vector3& a, const Vector3& b);

    /** Returns cross product value */
    Vector3 crossProduct(const Vector3& a, const Vector3& b);
}

#endif