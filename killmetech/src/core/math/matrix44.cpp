#include "matrix44.h"
#include "vector3.h"
#include "math.h"
#include <cmath>
#include <cassert>

namespace killme
{
    const Matrix44 Matrix44::IDENTITY = {
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1
    };

	Matrix44::Matrix44()
		: Matrix44(Matrix44::IDENTITY)
	{
	}

    Matrix44::Matrix44(
        float n00, float n01, float n02, float n03,
        float n10, float n11, float n12, float n13,
        float n20, float n21, float n22, float n23,
        float n30, float n31, float n32, float n33
        )
		: m_()
    {
        m_[0][0] = n00;
        m_[0][1] = n01;
        m_[0][2] = n02;
        m_[0][3] = n03;

        m_[1][0] = n10;
        m_[1][1] = n11;
        m_[1][2] = n12;
        m_[1][3] = n13;

        m_[2][0] = n20;
        m_[2][1] = n21;
        m_[2][2] = n22;
        m_[2][3] = n23;

        m_[3][0] = n30;
        m_[3][1] = n31;
        m_[3][2] = n32;
        m_[3][3] = n33;
    }

    Matrix44::Matrix44(std::initializer_list<float> il)
		: m_()
    {
		*this = il;
    }

    Matrix44& Matrix44::operator =(std::initializer_list<float> il)
    {
        assert(il.size() == 16 && "Invalid initializer list.");

		auto it = std::begin(il);
		for (int r = 0; r < 4; ++r)
		{
			for (int c = 0; c < 4; ++c)
			{
				m_[r][c] = *it;
				++it;
			}
		}
        return *this;
    }

    const float& Matrix44::operator ()(size_t r, size_t c) const
    {
        assert(r < 4 && c < 4 && "Index out of range.");
        return m_[r][c];
    }

    float& Matrix44::operator ()(size_t r, size_t c)
    {
        return const_cast<float&>(static_cast<const Matrix44&>(*this)(r, c));
    }

    bool operator ==(const Matrix44& a, const Matrix44& b)
    {
        return equalf(a(0, 0), b(0, 0)) && equalf(a(0, 1), b(0, 1)) && equalf(a(0, 2), b(0, 2)) && equalf(a(0, 3), b(0, 3))
            && equalf(a(1, 0), b(1, 0)) && equalf(a(1, 1), b(1, 1)) && equalf(a(1, 2), b(1, 2)) && equalf(a(1, 3), b(1, 3))
            && equalf(a(2, 0), b(2, 0)) && equalf(a(2, 1), b(2, 1)) && equalf(a(2, 2), b(2, 2)) && equalf(a(2, 3), b(2, 3))
            && equalf(a(3, 0), b(3, 0)) && equalf(a(3, 1), b(3, 1)) && equalf(a(3, 2), b(3, 2)) && equalf(a(3, 3), b(3, 3));
    }

    bool operator !=(const Matrix44& a, const Matrix44& b)
    {
        return !(a == b);
    }

    const Matrix44 operator +(const Matrix44& m)
    {
        return m;
    }

    const Matrix44 operator -(const Matrix44& m)
    {
        return {
            -m(0, 0), -m(0, 1), -m(0, 2), -m(0, 3),
            -m(1, 0), -m(1, 1), -m(1, 2), -m(1, 3),
            -m(2, 0), -m(2, 1), -m(2, 2), -m(2, 3),
            -m(3, 0), -m(3, 1), -m(3, 2), -m(3, 3)
        };
    }

    const Matrix44 operator +(const Matrix44& a, const Matrix44& b)
    {
        return {
            a(0, 0) + b(0, 0), a(0, 1) + b(0, 1), a(0, 2) + b(0, 2), a(0, 3) + b(0, 3),
            a(1, 0) + b(1, 0), a(1, 1) + b(1, 1), a(1, 2) + b(1, 2), a(1, 3) + b(1, 3),
            a(2, 0) + b(2, 0), a(2, 1) + b(2, 1), a(2, 2) + b(2, 2), a(2, 3) + b(2, 3),
            a(3, 0) + b(3, 0), a(3, 1) + b(3, 1), a(3, 2) + b(3, 2), a(3, 3) + b(3, 3)
        };
    }

    const Matrix44 operator -(const Matrix44& a, const Matrix44& b)
    {
        return {
            a(0, 0) - b(0, 0), a(0, 1) - b(0, 1), a(0, 2) - b(0, 2), a(0, 3) - b(0, 3),
            a(1, 0) - b(1, 0), a(1, 1) - b(1, 1), a(1, 2) - b(1, 2), a(1, 3) - b(1, 3),
            a(2, 0) - b(2, 0), a(2, 1) - b(2, 1), a(2, 2) - b(2, 2), a(2, 3) - b(2, 3),
            a(3, 0) - b(3, 0), a(3, 1) - b(3, 1), a(3, 2) - b(3, 2), a(3, 3) - b(3, 3)
        };
    }

    const Matrix44 operator *(const Matrix44& a, const Matrix44& b)
    {
        return {
            a(0, 0) * b(0, 0) + a(0, 1) * b(1, 0) + a(0, 2) * b(2, 0) + a(0, 3) * b(3, 0),
            a(0, 0) * b(0, 1) + a(0, 1) * b(1, 1) + a(0, 2) * b(2, 1) + a(0, 3) * b(3, 1),
            a(0, 0) * b(0, 2) + a(0, 1) * b(1, 2) + a(0, 2) * b(2, 2) + a(0, 3) * b(3, 2),
            a(0, 0) * b(0, 3) + a(0, 1) * b(1, 3) + a(0, 2) * b(2, 3) + a(0, 3) * b(3, 3),

            a(1, 0) * b(0, 0) + a(1, 1) * b(1, 0) + a(1, 2) * b(2, 0) + a(1, 3) * b(3, 0),
            a(1, 0) * b(0, 1) + a(1, 1) * b(1, 1) + a(1, 2) * b(2, 1) + a(1, 3) * b(3, 1),
            a(1, 0) * b(0, 2) + a(1, 1) * b(1, 2) + a(1, 2) * b(2, 2) + a(1, 3) * b(3, 2),
            a(1, 0) * b(0, 3) + a(1, 1) * b(1, 3) + a(1, 2) * b(2, 3) + a(1, 3) * b(3, 3),

            a(2, 0) * b(0, 0) + a(2, 1) * b(1, 0) + a(2, 2) * b(2, 0) + a(2, 3) * b(3, 0),
            a(2, 0) * b(0, 1) + a(2, 1) * b(1, 1) + a(2, 2) * b(2, 1) + a(2, 3) * b(3, 1),
            a(2, 0) * b(0, 2) + a(2, 1) * b(1, 2) + a(2, 2) * b(2, 2) + a(2, 3) * b(3, 2),
            a(2, 0) * b(0, 3) + a(2, 1) * b(1, 3) + a(2, 2) * b(2, 3) + a(2, 3) * b(3, 3),

            a(3, 0) * b(0, 0) + a(3, 1) * b(1, 0) + a(3, 2) * b(2, 0) + a(3, 3) * b(3, 0),
            a(3, 0) * b(0, 1) + a(3, 1) * b(1, 1) + a(3, 2) * b(2, 1) + a(3, 3) * b(3, 1),
            a(3, 0) * b(0, 2) + a(3, 1) * b(1, 2) + a(3, 2) * b(2, 2) + a(3, 3) * b(3, 2),
            a(3, 0) * b(0, 3) + a(3, 1) * b(1, 3) + a(3, 2) * b(2, 3) + a(3, 3) * b(3, 3)
        };
    }

    const Matrix44 operator *(const Matrix44& m, float k)
    {
        return {
            m(0, 0) * k, m(0, 1) * k, m(0, 2) * k, m(0, 3) * k,
            m(1, 0) * k, m(1, 1) * k, m(1, 2) * k, m(1, 3) * k,
            m(2, 0) * k, m(2, 1) * k, m(2, 2) * k, m(2, 3) * k,
            m(3, 0) * k, m(3, 1) * k, m(3, 2) * k, m(3, 3) * k
        };
    }

    const Matrix44 operator *(float k, const Matrix44& m)
    {
        return m * k;
    }

    const Matrix44 operator /(const Matrix44& m, float k)
    {
        assert(!equalf(k, 0) && "Division by zero error.");
        const auto invK = 1 / k;
        return m * invK;
    }

    Matrix44& operator +=(Matrix44& a, const Matrix44& b)
    {
        a = a + b;
        return a;
    }

    Matrix44& operator -=(Matrix44& a, const Matrix44& b)
    {
        a = a - b;
        return a;
    }

    Matrix44& operator *=(Matrix44& a, const Matrix44& b)
    {
        a = a * b;
        return a;
    }

    Matrix44& operator *=(Matrix44& m, float k)
    {
        m = m * k;
        return m;
    }

    Matrix44& operator /=(Matrix44& m, float k)
    {
        assert(!equalf(k, 0) && "Division by zero error.");
        m = m / k;
        return m;
    }

    Matrix44 transpose(const Matrix44& m)
    {
        return {
            m(0, 0), m(1, 0), m(2, 0), m(3, 0),
            m(0, 1), m(1, 1), m(2, 1), m(3, 1),
            m(0, 2), m(1, 2), m(2, 2), m(3, 2),
            m(0, 3), m(1, 3), m(2, 3), m(3, 3)
        };
    }

    float determinant(const Matrix44& m)
    {
        return (m(0, 0) * m(1, 1) - m(0, 1) * m(1, 0)) * (m(2, 2) * m(3, 3) - m(2, 3) * m(3, 2)) -
            (m(0, 0) * m(1, 2) - m(0, 2) * m(1, 0)) * (m(2, 1) * m(3, 3) - m(2, 3) * m(3, 1)) +
            (m(0, 0) * m(1, 3) - m(0, 3) * m(1, 0)) * (m(2, 1) * m(3, 2) - m(2, 2) * m(3, 1)) +
            (m(0, 1) * m(1, 2) - m(0, 2) * m(1, 1)) * (m(2, 0) * m(3, 3) - m(2, 3) * m(3, 0)) -
            (m(0, 1) * m(1, 3) - m(0, 3) * m(1, 1)) * (m(2, 0) * m(3, 2) - m(2, 2) * m(3, 0)) +
            (m(0, 2) * m(1, 3) - m(0, 3) * m(1, 2)) * (m(2, 0) * m(3, 1) - m(2, 1) * m(3, 0));
    }

    Matrix44 inverse(const Matrix44& m)
    {
        return inverse(m, determinant(m));
    }

    Matrix44 inverse(const Matrix44& m, float det)
    {
        assert(!equalf(det, 0) && "Not exist inversed matrix.");
        const auto invDet = 1 / det;
        return {
            invDet * (m(1, 1) * (m(2, 2) * m(3, 3) - m(2, 3) * m(3, 2)) + m(1, 2) * (m(2, 3) * m(3, 1) - m(2, 1) * m(3, 3)) + m(1, 3) * (m(2, 1) * m(3, 2) - m(2, 2) * m(3, 1))),
            invDet * (m(2, 1) * (m(0, 2) * m(3, 3) - m(0, 3) * m(3, 2)) + m(2, 2) * (m(0, 3) * m(3, 1) - m(0, 1) * m(3, 3)) + m(2, 3) * (m(0, 1) * m(3, 2) - m(0, 2) * m(3, 1))),
            invDet * (m(3, 1) * (m(0, 2) * m(1, 3) - m(0, 3) * m(1, 2)) + m(3, 2) * (m(0, 3) * m(1, 1) - m(0, 1) * m(1, 3)) + m(3, 3) * (m(0, 1) * m(1, 2) - m(0, 2) * m(1, 1))),
            invDet * (m(0, 1) * (m(1, 3) * m(2, 2) - m(1, 2) * m(2, 3)) + m(0, 2) * (m(1, 1) * m(2, 3) - m(1, 3) * m(2, 1)) + m(0, 3) * (m(1, 2) * m(2, 1) - m(1, 1) * m(2, 2))),

            invDet * (m(1, 2) * (m(2, 0) * m(3, 3) - m(2, 3) * m(3, 0)) + m(1, 3) * (m(2, 2) * m(3, 0) - m(2, 0) * m(3, 2)) + m(1, 0) * (m(2, 3) * m(3, 2) - m(2, 2) * m(3, 3))),
            invDet * (m(2, 2) * (m(0, 0) * m(3, 3) - m(0, 3) * m(3, 0)) + m(2, 3) * (m(0, 2) * m(3, 0) - m(0, 0) * m(3, 2)) + m(2, 0) * (m(0, 3) * m(3, 2) - m(0, 2) * m(3, 3))),
            invDet * (m(3, 2) * (m(0, 0) * m(1, 3) - m(0, 3) * m(1, 0)) + m(3, 3) * (m(0, 2) * m(1, 0) - m(0, 0) * m(1, 2)) + m(3, 0) * (m(0, 3) * m(1, 2) - m(0, 2) * m(1, 3))),
            invDet * (m(0, 2) * (m(1, 3) * m(2, 0) - m(1, 0) * m(2, 3)) + m(0, 3) * (m(1, 0) * m(2, 2) - m(1, 2) * m(2, 0)) + m(0, 0) * (m(1, 2) * m(2, 3) - m(1, 3) * m(2, 2))),

            invDet * (m(1, 3) * (m(2, 0) * m(3, 1) - m(2, 1) * m(3, 0)) + m(1, 0) * (m(2, 1) * m(3, 3) - m(2, 3) * m(3, 1)) + m(1, 1) * (m(2, 3) * m(3, 0) - m(2, 0) * m(3, 3))),
            invDet * (m(2, 3) * (m(0, 0) * m(3, 1) - m(0, 1) * m(3, 0)) + m(2, 0) * (m(0, 1) * m(3, 3) - m(0, 3) * m(3, 1)) + m(2, 1) * (m(0, 3) * m(3, 0) - m(0, 0) * m(3, 3))),
            invDet * (m(3, 3) * (m(0, 0) * m(1, 1) - m(0, 1) * m(1, 0)) + m(3, 0) * (m(0, 1) * m(1, 3) - m(0, 3) * m(1, 1)) + m(3, 1) * (m(0, 3) * m(1, 0) - m(0, 0) * m(1, 3))),
            invDet * (m(0, 3) * (m(1, 1) * m(2, 0) - m(1, 0) * m(2, 1)) + m(0, 0) * (m(1, 3) * m(2, 1) - m(1, 1) * m(2, 3)) + m(0, 1) * (m(1, 0) * m(2, 3) - m(1, 3) * m(2, 0))),

            invDet * (m(1, 0) * (m(2, 2) * m(3, 1) - m(2, 1) * m(3, 2)) + m(1, 1) * (m(2, 0) * m(3, 2) - m(2, 2) * m(3, 0)) + m(1, 2) * (m(2, 1) * m(3, 0) - m(2, 0) * m(3, 1))),
            invDet * (m(2, 0) * (m(0, 2) * m(3, 1) - m(0, 1) * m(3, 2)) + m(2, 1) * (m(0, 0) * m(3, 2) - m(0, 2) * m(3, 0)) + m(2, 2) * (m(0, 1) * m(3, 0) - m(0, 0) * m(3, 1))),
            invDet * (m(3, 0) * (m(0, 2) * m(1, 1) - m(0, 1) * m(1, 2)) + m(3, 1) * (m(0, 0) * m(1, 2) - m(0, 2) * m(1, 0)) + m(3, 2) * (m(0, 1) * m(1, 0) - m(0, 0) * m(1, 1))),
            invDet * (m(0, 0) * (m(1, 1) * m(2, 2) - m(1, 2) * m(2, 1)) + m(0, 1) * (m(1, 2) * m(2, 0) - m(1, 0) * m(2, 2)) + m(0, 2) * (m(1, 0) * m(2, 1) - m(1, 1) * m(2, 0)))
        };
    }

    Matrix44 makeTransformMatrix(const Vector3& scale, const Quaternion& rot, const Vector3& trans)
    {
		auto m = Matrix44::IDENTITY;

        // Apply scaling
		m(0, 0) = scale.x;
		m(1, 1) = scale.y;
		m(2, 2) = scale.z;

        // Apply rotation
		m *= to<Matrix44>(rot);

        // Apply translation
        m(3, 0) = trans.x;
        m(3, 1) = trans.y;
        m(3, 2) = trans.z;

        return m;
    }

	Matrix44 makeProjectionMatrix(float fovX, float aspect, float zn, float zf)
    {
        assert(fovX > 0 && "Invalid X fov.");
        assert(aspect > 0 && "Invalid aspect rate.");
        assert(zn < zf && "Invalid Near and far Z value.");

        const auto fovY = fovX / aspect;
        const auto n11 = 1 / std::tan(fovX * 0.5f);
		//const auto n22 = 1 / std::tan(fovY * 0.5f);
        const auto n22 = n11 / aspect;
        const auto n33 = zf / (zf - zn);
        const auto n43 = -n33 * zn;
        return {
            n11, 0, 0, 0,
            0, n22, 0, 0,
            0, 0, n33, 1,
            0, 0, n43, 0
        };
    }
}