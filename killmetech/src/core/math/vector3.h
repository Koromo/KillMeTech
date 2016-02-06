#ifndef _KILLME_VECTOR3_H_
#define _KILLME_VECTOR3_H_

namespace killme
{
    class Quaternion;

    /** The 3D vector */
    class Vector3
    {
    public:
        /** The elements */
        float x, y, z;

        /** Constructs the zero vector */
        Vector3();

        /** Constructs with initial values */
        Vector3(float nx, float ny, float nz);

        /** Accessor */
        const float& operator [](size_t i) const;
        float& operator [](size_t i);

        /** Specific vector definitions */
        static const Vector3 ZERO;
        static const Vector3 UNIT_X;
        static const Vector3 UNIT_Y;
        static const Vector3 UNIT_Z;
    };

    /** Operator overloads */
    bool operator ==(const Vector3& a, const Vector3& b);
    bool operator !=(const Vector3& a, const Vector3& b);

    const Vector3 operator +(const Vector3& v);
    const Vector3 operator -(const Vector3& v);

    const Vector3 operator +(const Vector3& a, const Vector3& b);
    const Vector3 operator -(const Vector3& a, const Vector3& b);
    const Vector3 operator *(const Vector3& v, float k);
    const Vector3 operator *(float k, const Vector3& v);
    const Vector3 operator *(const Quaternion& q, const Vector3& v);
    const Vector3 operator /(const Vector3& v, float k);

    Vector3& operator +=(Vector3& a, const Vector3& b);
    Vector3& operator -=(Vector3& a, const Vector3& b);
    Vector3& operator *=(Vector3& v, float k);
    Vector3& operator /=(Vector3& v, float k);

    /** Scale the vector */
    Vector3 scale(const Vector3& v, const Vector3& k);

    /** Returns the norm */
    float norm(const Vector3& v);

    /** Returns the normalized vector */
    Vector3 normalize(const Vector3& v);

    /** Returns the dot product value */
    float dotProduct(const Vector3& a, const Vector3& b);

    /** Returns the cross product value */
    Vector3 crossProduct(const Vector3& a, const Vector3& b);
}

#endif