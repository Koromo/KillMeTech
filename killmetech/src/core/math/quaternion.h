#ifndef _KILLME_QUATERNION_H_
#define _KILLME_QUATERNION_H_

namespace killme
{
    class Vector3;

    /** The Quaternion */
    class Quaternion
    {
    public:
        /** The elements */
        float w, x, y, z;

        /** Constructs the identical quaternion */
        Quaternion();

        /** Constructs with initial values */
        Quaternion(float nw, float nx, float ny, float nz);

        /** Accessor */
        const float& operator [](size_t i) const;
        float& operator [](size_t i);

        /** Specific quaternion definition */
        static const Quaternion IDENTITY;
    };

    /** Operator overloads */
    bool operator ==(const Quaternion& a, const Quaternion& b);
    bool operator !=(const Quaternion& a, const Quaternion& b);

    const Quaternion operator +(const Quaternion& q);
    const Quaternion operator -(const Quaternion& q);

    const Quaternion operator +(const Quaternion& a, const Quaternion& b);
    const Quaternion operator -(const Quaternion& a, const Quaternion& b);
    const Quaternion operator *(const Quaternion& a, const Quaternion& b);
    const Quaternion operator *(const Quaternion& q, float k);

	Quaternion& operator +=(Quaternion& a, const Quaternion& b);
	Quaternion& operator -=(Quaternion& a, const Quaternion& b);
    Quaternion& operator *=(Quaternion& a, const Quaternion& b);
    Quaternion& operator *=(Quaternion& q, float k);

    /** Returns the norm */
    float norm(const Quaternion& q);

    /** Returns the normalized quaternion */
    Quaternion normalize(const Quaternion& q);

    /** Returns the conjugated quaternion */
    Quaternion conjugate(const Quaternion& q);

    /** Returns the inversed quaternion */
    Quaternion inverse(const Quaternion& q);

    /** Returns the dot product value */
    float dotProduct(const Quaternion& a, const Quaternion& b);

    /** Create the quaternion from a rotation axis and an angle[rad] */
    Quaternion makeQuaternion(const Vector3& axis, float angle);

    /** Returns the rotation axis */
    Vector3 axis(const Quaternion& q);

    /** Returns the rotation angle[rad] */
    float angle(const Quaternion& q);
}

#endif