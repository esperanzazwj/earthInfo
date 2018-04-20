#include "plane.hpp"
#include "bounding-box.hpp"

namespace ss
{
    namespace math
    {
        auto Plane::side_of(Point3 const& p) const -> Side
        {
            float dist = abs_distance_to(p);
            return (
                dist > 0.0f ? Plane::POSITIVE_SIDE :
                dist < 0.0f ? Plane::NEGATIVE_SIDE :
                Plane::INTERSECTING_PLANE);
        }

        auto Plane::side_of(BoundingBox const& bb) const -> Side
        {
            if (bb.isNull()) return Plane::NONE;
            if (bb.isInfinite()) return Plane::INTERSECTING_PLANE;

            auto dist = signed_distance_to(bb.getCenter());
            auto absd = normal.absDotProduct(bb.getHalfSize());
            return (
                dist > +absd ? Plane::POSITIVE_SIDE :
                dist < -absd ? Plane::NEGATIVE_SIDE :
                Plane::INTERSECTING_PLANE);
        }

        float Plane::normalize()
        {
            auto len = normal.length();
            d /= len;
            normal /= len;
            return len;
        }
    }
}

