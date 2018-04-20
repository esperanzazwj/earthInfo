#pragma once
#include "point3.hpp"
#include "vector3.hpp"
#include "forward.hpp"
#include <cmath>

namespace ss
{
    namespace math
    {
        struct Plane
        {
            Vector3 normal{};
            float d{};

            enum Side
            {
                POSITIVE_SIDE,
                NEGATIVE_SIDE,
                INTERSECTING_PLANE,
                NONE,
            };

            Plane() = default;
            Plane(Vector3 const& normal, float offset) : normal{normal}, d{-offset} {}

            Plane(Vector3 const& normal, Point3 const& p)
                : normal{normal}
                , d{-normal.dotProduct(p)}
            {}
            Plane(Point3 const& a, Point3 const& b, Point3 const& c)
                : normal{(b-a).crossProduct(c-a).hat()}
                , d{-normal.dotProduct(a)}
            {}

            Side side_of(Point3 const& p) const;
            Side side_of(BoundingBox const& bb) const;

            float signed_distance_to(Point3 const& p) const { return normal.dotProduct(p) + d; }
            float abs_distance_to(Point3 const& p) const { return std::abs(signed_distance_to(p)); }

            float normalize();
        };

        inline bool operator == (Plane const& a, Plane const& b) { return (a.d == b.d && a.normal == b.normal); }
        inline bool operator != (Plane const& a, Plane const& b) { return (a.d != b.d || b.normal != b.normal); }
    }
}

