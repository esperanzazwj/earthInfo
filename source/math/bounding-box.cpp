#include "bounding-box.hpp"
#include "ray.hpp"
#include "sphere.hpp"
#include "plane.hpp"
#include <cmath>

namespace ss
{
    namespace math
    {
        bool BoundingBox::intersect(Ray const& ray, float & t0, float & t1) const
        {
            auto near = (min_point - ray.origin) / ray.dir;
            auto  far = (max_point - ray.origin) / ray.dir;
            swap_unless_min_max(near, far);

            t0 = max(Vector4{near, ray.near});
            t1 = min(Vector4{ far, ray. far});
            return (t0 <= t1);
        }

        bool BoundingBox::intersect(Sphere const& sphere) const
        {
            return sphere.intersect(*this);
        }

        bool BoundingBox::intersect(Plane const& plane) const
        {
            return (plane.side_of(*this) == Plane::INTERSECTING_PLANE);
        }
    }
}

