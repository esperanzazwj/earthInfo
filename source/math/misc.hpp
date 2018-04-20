#pragma once
#include "point3.hpp"
#include "vector3.hpp"
#include "constants.hpp"
#include <cmath>
#include <algorithm>

namespace ss
{
    namespace math
    {
        inline namespace misc
        {
            inline float mix(float a, float b, float x)
            {
                return x * b + (1-x) * a;
            }

            inline auto Clamp(float x, float from, float to)
            {
                return std::min(std::max(x, from), to);
            }

            inline float Distance(Point3 const& p1, Point3 const& p2)
            {
                return (p1 - p2).length();
            }

            inline float DistanceSquared(Point3 const& p1, Point3 const& p2)
            {
                return (p1 - p2).squaredLength();
            }

            inline Vector3 SphericalDirection(float sintheta, float costheta, float phi)
            {
                return Vector3(sintheta * std::cos(phi), sintheta * std::sin(phi), costheta);
            }

            inline Vector3 SphericalDirection(
                float sintheta,
                float costheta,
                float phi,
                Vector3 const& x,
                Vector3 const& y,
                Vector3 const& z)
            {
                return sintheta * std::cos(phi) * x + sintheta * std::sin(phi) * y + costheta * z;
            }

            inline float SphericalTheta(Vector3 const& v)
            {
                return std::acos(v.z);
            }

            inline float SphericalPhi(Vector3 const& v)
            {
                return std::atan2(v.y, v.x) + constants::pi;
            }
        }
    }
}

