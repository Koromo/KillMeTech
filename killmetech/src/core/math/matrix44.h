#ifndef _KILLME_MATRIX44_H_
#define _KILLME_MATRIX44_H_

#include "quaternion.h"
#include "../utility.h"
#include <array>
#include <initializer_list>

namespace killme
{
    class Vector3;

    /** The 4*4 matrix */
    class Matrix44
    {
    private:
		std::array<std::array<float, 4>, 4> m_;

    public:
        /** Constructs the identical matrix */
        Matrix44();

        /** Constructs with initial values */
        Matrix44(
			float n00, float n01, float n02, float n03,
            float n10, float n11, float n12, float n13,
            float n20, float n21, float n22, float n23,
            float n30, float n31, float n32, float n33
			);

        /** Constructs with a initializer list */
        Matrix44(std::initializer_list<float> il);

        /** Assignment operator with a initializer list */
        Matrix44& operator =(std::initializer_list<float> il);

        /** Accessor */
        const float& operator ()(size_t r, size_t c) const;
        float& operator ()(size_t r, size_t c);

        /** Specific matrix definition */
        static const Matrix44 IDENTITY;
    };

    /** Operator overloads */
    bool operator ==(const Matrix44& a, const Matrix44& b);
    bool operator !=(const Matrix44& a, const Matrix44& b);

    const Matrix44 operator +(const Matrix44& m);
    const Matrix44 operator -(const Matrix44& m);

    const Matrix44 operator +(const Matrix44& a, const Matrix44& b);
    const Matrix44 operator -(const Matrix44& a, const Matrix44& b);
    const Matrix44 operator *(const Matrix44& a, const Matrix44& b);
    const Matrix44 operator *(const Matrix44& m, float k);
    const Matrix44 operator *(float k, const Matrix44& m);
    const Matrix44 operator /(const Matrix44& m, float k);

    Matrix44& operator +=(Matrix44& a, const Matrix44& b);
    Matrix44& operator -=(Matrix44& a, const Matrix44& b);
    Matrix44& operator *=(Matrix44& a, const Matrix44& b);
    Matrix44& operator *=(Matrix44& m, float k);
    Matrix44& operator /=(Matrix44& m, float k);

    /** Returns the transposed matrix */
    Matrix44 transpose(const Matrix44& m);

    /** Returns the determinant */
    float determinant(const Matrix44& m);

    /** Returns the inversed matrix */
    Matrix44 inverse(const Matrix44& m);
    Matrix44 inverse(const Matrix44& m, float det);

    /** Convert the Quaternion to the Matrix44 */
    template <>
    inline Matrix44 to<Matrix44, Quaternion>(const Quaternion& q)
    {
        const auto w2 = q.w * 2;
        const auto x2 = q.x * 2;
        const auto y2 = q.y * 2;
        const auto z2 = q.z * 2;

        auto m = Matrix44::IDENTITY;

        m(0, 0) = 1 - y2 * q.y - z2 * q.z;
        m(0, 1) = x2 * q.y + w2 * q.z;
        m(0, 2) = x2 * q.z - w2 * q.y;

        m(1, 0) = x2 * q.y - w2 * q.z;
        m(1, 1) = 1 - x2 * q.x - z2 * q.z;
        m(1, 2) = y2 * q.z + w2 * q.x;

        m(2, 0) = x2 * q.z + w2 * q.y;
        m(2, 1) = y2 * q.z - w2 * q.x;
        m(2, 2) = 1 - x2 * q.x - y2 * q.y;

        return m;
    }

    /** Create the geometric matrix */
    Matrix44 makeTransformMatrix(const Vector3& scale, const Quaternion& rot, const Vector3& trans);
    Matrix44 makeProjectionMatrix(float fovX, float aspect, float zn, float zf); /// Aspect rate calclated by w/h
}

#endif