#include "ray.hpp"
#include "plane.hpp"
#include "bounding-box.hpp"
#include "sphere.hpp"
#include <cmath>

namespace ss
{
    namespace math
    {
        bool Ray::intersect(Sphere const& sphere, float & t1, float & t2) const
            {
            auto oc = origin - sphere.center;
            auto a = dir.squaredLength();  // XXX: always 1.0f ?
            auto b = oc.dotProduct(dir) * 2.0f;
            auto c = oc.squaredLength() - sphere.radius*sphere.radius;
            auto d = b * b - 4.0f * a * c;

            if (d < -1e-6f) return false;

            if (d < 1e-6f) {
                t1 = -0.5f*b/a;
                t2 = t1;
            } else {
                t1 = -0.5f * (b + std::sqrt(d)) / a;
                t2 = -0.5f * (b - std::sqrt(d)) / a;
                if (t1 > t2) std::swap(t1, t2);
            }

            return true;
        }


        bool Ray::intersect(BoundingBox const& bbox, float & t0,float & t1) const
        {
            return bbox.intersect(*this,t0,t1);
        }

        bool Ray::intersect(Plane const& plane, float & t) const
            {
            assert(near <= far);

            if (plane.side_of(origin) == Plane::INTERSECTING_PLANE) {
                t = 0;
                return true;
            }

            float nd = plane.normal.dotProduct(dir);
            if (std::abs(nd) < 1e-6f) return false;

            t = -plane.signed_distance_to(origin) / nd;
            return (near <= t && t <= far);
        }
    }
}

