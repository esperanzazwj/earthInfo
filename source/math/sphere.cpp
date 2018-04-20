#include "sphere.hpp"
#include "plane.hpp"
#include "bounding-box.hpp"

namespace ss
{
    namespace math
    {
        bool Sphere::intersect(BoundingBox const& bb) const
        {
            if (bb.isNull()) return false;
            if (bb.isInfinite()) return true;

            auto& min = bb.min_point;
            auto& max = bb.max_point;

            // Arvo's algorithm
            auto d = 0.0f;
            for (int i = 0; i < 3; i++) {
                if (center[i] < min[i]) {
                    auto s = center[i] - min[i];
                    d += s * s;
                }
                else if (center[i] > max[i]) {
                    auto s = center[i] - max[i];
                    d += s * s;
                }
            }

            return d <= radius * radius;
        }

        bool Sphere::intersect(Plane const& p) const
        {
            return (p.abs_distance_to(center) <= radius);
        }

        void Sphere::merge(Sphere const& a)
        {
            auto dc = a.center - center;
            auto dr = a.radius - radius;
            auto dclen = dc.length();

            if (dr >= dclen) {
                // One fully contains the other
                if (dr <= 0.0f) return;

                center = a.center;
                radius = a.radius;
                return;
            }

            if (dclen + dr > 0.0f) {
                auto t = (dclen + dr) / (2.0f * dclen);
                center += dc * t;
            }
            radius = 0.5f * (dclen + radius + a.radius);
        }
    }
}

