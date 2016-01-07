#ifndef _KILLME_MATRIX44_H_
#define _KILLME_MATRIX44_H_

#include <array>
#include <initializer_list>

namespace killme
{
    class Vector3;
    class Quaternion;

    /** 4*4 matrix */
    class Matrix44
    {
    private:
		std::array<std::array<float, 4>, 4> m_;

    public:
        /** Construct identical matrix */
        Matrix44();

        /** Construct with a initial value */
        Matrix44(
			float n00, float n01, float n02, float n03,
            float n10, float n11, float n12, float n13,
            float n20, float n21, float n22, float n23,
            float n30, float n31, float n32, float n33
			);

        /** Construct with a initializer list */
        Matrix44(std::initializer_list<float> il);

        /** Assignment operator that take a initializer list */
        Matrix44& operator=(std::initializer_list<float> il);

        /** Accessor */
        const float& operator()(size_t r, size_t c) const;
        float& operator()(size_t r, size_t c);

        /** Specific matrix definition */
        static const Matrix44 IDENTITY;
    };

    /** Operator overloads */
    bool operator==(const Matrix44& a, const Matrix44& b);
    bool operator!=(const Matrix44& a, const Matrix44& b);

    const Matrix44 operator+(const Matrix44& m);
    const Matrix44 operator-(const Matrix44& m);

    const Matrix44 operator+(const Matrix44& a, const Matrix44& b);
    const Matrix44 operator-(const Matrix44& a, const Matrix44& b);
    const Matrix44 operator*(const Matrix44& a, const Matrix44& b);
    const Matrix44 operator*(const Matrix44& m, float k);
    const Matrix44 operator*(float k, const Matrix44& m);
    const Matrix44 operator/(const Matrix44& m, float k);

    Matrix44& operator+=(Matrix44& a, const Matrix44& b);
    Matrix44& operator-=(Matrix44& a, const Matrix44& b);
    Matrix44& operator*=(Matrix44& a, const Matrix44& b);
    Matrix44& operator*=(Matrix44& m, float k);
    Matrix44& operator/=(Matrix44& m, float k);

    /** Returns transposed matrix */
    Matrix44 transpose(const Matrix44& m);

    /** Returns determinant of matrix */
    float determinant(const Matrix44& m);

    /** Returns inversed matrix */
    Matrix44 inverse(const Matrix44& m);
    Matrix44 inverse(const Matrix44& m, float det);

    /** Convert Quaternion to Matrix44 */
    Matrix44 toMatrix44(const Quaternion& q);

    /** Create geometric matrix */
    Matrix44 makeTransformMatrix(const Vector3& scale, const Quaternion& rot, const Vector3& trans);
    Matrix44 makeProjectionMatrix(float fovX, float aspect, float zn, float zf); /// Aspect rate calclated by w/h
}

#endif