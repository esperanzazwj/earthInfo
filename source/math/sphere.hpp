#pragma once
#include "point3.hpp"
#include "forward.hpp"

namespace ss
{
    namespace math
    {
        struct Sphere
        {
            Point3 center{};
            float radius{};

            Sphere() = default;
            Sphere(Point3 center, float radius) : center{center}, radius{radius} {}

            bool contains(Point3 const& p) const
            {
                return ((p - center).squaredLength() <= radius*radius);
            }

            bool intersect(Sphere const& a) const
            {
                auto r = a.radius + radius;
                auto d = a.center - center;
                return (d.squaredLength() <= r*r);
            }

            bool intersect(BoundingBox const& bb) const;
            bool intersect(Plane const& p) const;

            void merge(Sphere const& a);

            [[deprecated("Use sphere.radius instead of sphere.getRadius().")]]
            auto getRadius() const { return radius; }

            [[deprecated("Use sphere.center instead of sphere.getCenter().")]]
            auto& getCenter() const { return center; }

            [[deprecated("Use sphere.radius = x instead of sphere.setRadius(x).")]]
            void setRadius(float radius) { this->radius = radius; }

            [[deprecated("Use sphere.center = x instead of sphere.setCenter(x).")]]
            void setCenter(Point3 const& center) { this->center = center; }
        };
    }
}

