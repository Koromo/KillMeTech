#ifndef _KILLME_BULLETSUPPORT_H_
#define _KILLME_BULLETSUPPORT_H_

#include "../core/math/vector3.h"
#include "../core/math/quaternion.h"
#include "../core/utility.h"
#include <LinearMath/btVector3.h>
#include <LinearMath/btQuaternion.h>

namespace killme
{
    /** Convert 3D vector */
    template <>
    inline btVector3 to<btVector3, Vector3>(const Vector3& v)
    {
        return btVector3(v.x, v.y, v.z);
    }

    /** Convert 3D vector */
    template <>
    inline Vector3 to<Vector3, btVector3>(const btVector3& v)
    {
        return{ v.x(), v.y(), v.z() };
    }

    /** Convert Quaternion */
    template <>
    inline btQuaternion to<btQuaternion, Quaternion>(const Quaternion& q)
    {
        return btQuaternion(q.x, q.y, q.z, q.w);
    }

    /** Convert Quaternion */
    template <>
    inline Quaternion to<Quaternion, btQuaternion>(const btQuaternion& q)
    {
        return{ q.w(), q.x(), q.y(), q.z() };
    }
}

#endif