#pragma once
#include "../../math/all.hpp"

namespace HW
{
    inline namespace math
    {
        using ss::math::Vector2;
        using ss::math::Vector3;
        using ss::math::Vector4;

        using ss::math::Matrix3;
        using ss::math::Matrix4;

        using ss::math::Ray;
        using ss::math::Quaternion;

        using ss::math::Point3;
        using ss::math::Plane;
        using ss::math::BoundingBox;
        using ss::math::Sphere;

        namespace constants = ss::math::constants;
        using namespace ss::math::coordinate_system;
        using namespace ss::math::misc;
    }
}

