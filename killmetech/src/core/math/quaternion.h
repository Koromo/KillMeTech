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

        /** Construct identical quaternion */
        Quaternion();

        /** Construct with a initial value */
        Quaternion(float nw, float nx, float ny, float nz);

        /** Accessor */
        const float& operator[](size_t i) const;
        float& operator[](size_t i);

        /** Specific quaternion definition */
        static const Quaternion IDENTITY;
    };

    /** Operator overloads */
    bool operator==(const Quaternion& a, const Quaternion& b);
    bool operator!=(const Quaternion& a, const Quaternion& b);

    const Quaternion operator+(const Quaternion& q);
    const Quaternion operator-(const Quaternion& q);

    const Quaternion operator+(const Quaternion& a, const Quaternion& b);
    const Quaternion operator-(const Quaternion& a, const Quaternion& b);
    const Quaternion operator*(const Quaternion& a, const Quaternion& b);
    const Quaternion operator*(const Quaternion& q, float k);

	Quaternion& operator+=(Quaternion& a, const Quaternion& b);
	Quaternion& operator-=(Quaternion& a, const Quaternion& b);
    Quaternion& operator*=(Quaternion& a, const Quaternion& b);
    Quaternion& operator*=(Quaternion& q, float k);

    /** Returns norm of quaternion */
    float norm(const Quaternion& q);

    /** Returns normalized quaternion */
    Quaternion normalize(const Quaternion& q);

    /** Returns conjugated quaternion */
    Quaternion conjugate(const Quaternion& q);

    /** Returns inversed quaternion */
    Quaternion inverse(const Quaternion& q);

    /** Returns dot product value */
    float dotProduct(const Quaternion& a, const Quaternion& b);

    /** Create quaternion from rotation axis and angle[rad] */
    Quaternion makeQuaternion(const Vector3& axis, float angle);

    /** Returns rotation axis */
    Vector3 axis(const Quaternion& q);

    /** Returns rotation angle[rad] */
    float angle(const Quaternion& q);
}

#endif