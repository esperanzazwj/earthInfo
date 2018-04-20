#pragma once
#include "point3.hpp"
#include "vector3.hpp"
#include "constants.hpp"
#include <cassert>

namespace ss
{
    namespace math
    {
        struct Ray
        {
            Point3 origin;
            Vector3 dir;
            float near{constants::eps};
            float far{constants::inf};

            Ray(Point3 origin, Vector3 dir) : origin{origin}, dir{dir} {}

            Ray(Point3 origin, Vector3 dir, float near, float far=constants::inf)
                : origin{origin}
                , dir{dir}
                , near{near}
                , far{far}
            {
                assert(near <= far);
            }

            Point3 at(float t) const { return origin + dir * t; }
            Point3 operator [] (float t) const { return at(t); }

            [[deprecated("Use a.at(t) instead of a.getPoint(t).")]]
            Point3 getPoint(float t) const { return at(t); }

            [[deprecated("Use a[t] instead of a(t).")]]
            Point3 operator () (float t) const { return at(t); }

            bool intersect(Plane const& plane, float & t) const;
            bool intersect(BoundingBox const& bb, float & t0,float & t1) const;
            bool intersect(Sphere const& sphere, float & t1, float & t2) const;
        };
    }
}

