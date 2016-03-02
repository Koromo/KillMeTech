#ifndef _KILLME_QUATERNION_H_
#define _KILLME_QUATERNION_H_

namespace killme
{
    class Vector3;

    /** Quaternion */
    class Quaternion
    {
    public:
        /** Elements */
        float w, x, y, z;

        /** Construct as the identical quaternion */
        Quaternion();

        /** Construct with initial values */
        Quaternion(float nw, float nx, float ny, float nz);

        /** Construct */
        Quaternion(const Quaternion&) = default;
        Quaternion(Quaternion&&) = default;

        /** Assignment operator */
        Quaternion& operator =(const Quaternion&) = default;
        Quaternion& operator =(Quaternion&&) = default;

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

    /** Return the norm */
    float norm(const Quaternion& q);

    /** Return the normalized quaternion */
    Quaternion normalize(const Quaternion& q);

    /** Return the conjugated quaternion */
    Quaternion conjugate(const Quaternion& q);

    /** Return the inversed quaternion */
    Quaternion inverse(const Quaternion& q);

    /** Return the dot product value */
    float dotProduct(const Quaternion& a, const Quaternion& b);

    /** Create the quaternion from a rotation axis and an angle[rad] */
    Quaternion makeQuaternion(const Vector3& axis, float angle);

    /** Return the rotation axis */
    Vector3 axis(const Quaternion& q);

    /** Return the rotation angle[rad] */
    float angle(const Quaternion& q);
}

#endif