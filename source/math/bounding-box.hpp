#pragma once
#include "point3.hpp"
#include "vector3.hpp"
#include "matrix3.hpp"
#include "matrix4.hpp"
#include "forward.hpp"
#include "constants.hpp"
#include <iostream>
#include <stdexcept>

#include "../util/unreachable.macro.hpp"

namespace ss
{
    namespace math
    {
        bool operator != (BoundingBox const& a, BoundingBox const& b);
        bool operator == (BoundingBox const& a, BoundingBox const& b);

        struct BoundingBox
        {
            enum struct Extent
            {
                null,
                finite,
                infinite,
            };

            Point3 min_point{};
            Point3 max_point{};
            Extent extent{Extent::null};

            BoundingBox() = default;
            BoundingBox(Point3 const& p1, Point3 const& p2)
                : min_point{min(p1, p2)}
                , max_point{max(p1, p2)}
                , extent{Extent::finite}
            {}

            bool isNull() const { return (extent == Extent::null); }
            bool isInfinite() const { return (extent == Extent::infinite); }
            bool isFinite() const { return (extent == Extent::finite); }

            void setNull() { extent = Extent::null; }
            void setInfinite() { extent = Extent::infinite; }
            void setFinite() { extent = Extent::finite; }

            float volume() const
            {
                switch (extent) {
                    case Extent::null: return 0.0f;
                    case Extent::infinite: return constants::inf;
                    case Extent::finite: {
                        auto d = max_point - min_point;
                        return d.x * d.y * d.z;
                    }
                }
                SS_UTIL_UNREACHABLE();
            }

            Vector3 getHalfSize() const { return getSize() * 0.5f; }
            Vector3 getSize() const
            {
                switch (extent)
                {
                    case Extent::null: return Vector3{};
                    case Extent::infinite: return Vector3{constants::inf};
                    case Extent::finite: return max_point - min_point;
                }
                SS_UTIL_UNREACHABLE();
            }

            [[deprecated("Use bb.max_point instead of bb.getMaximum()")]]
            Point3 const& getMaximum() const { return max_point; }

            [[deprecated("Use bb.min_point instead of bb.getMinimum()")]]
            Point3 const& getMinimum() const { return min_point; }

            void setMinimum(Point3 min_point)
            {
                extent = Extent::finite;
                this->min_point = min_point;
            }

            void setMaximum(Point3 max_point)
            {
                extent = Extent::finite;
                this->max_point = max_point;
            }

            void setExtents(Point3 max_point, Point3 min_point)
            {
                extent = Extent::finite;
                this->max_point = max_point;
                this->min_point = min_point;
            }

            void expand(Vector3 const& half_size)
            {
                min_point -= half_size;
                max_point += half_size;
            }

            Point3 getCenter() const
            {
                if (extent != Extent::finite)
                    throw std::logic_error{"Only finite bounding box has a center"};

                return mix(min_point, max_point, 0.5f);
            }

            [[deprecated("Use overlaps instead of Overlaps")]]
            bool Overlaps(BoundingBox const& a) const { return overlaps(a); }

            bool overlaps(BoundingBox const& a) const
            {
                if (isNull() || a.isInfinite()) return true;
                if (isInfinite() || a.isNull()) return false;

                bool x = (max_point.x >= a.min_point.x) && (min_point.x <= a.max_point.x);
                bool y = (max_point.y >= a.min_point.y) && (min_point.y <= a.max_point.y);
                bool z = (max_point.z >= a.min_point.z) && (min_point.z <= a.max_point.z);
                return (x && y && z);
            }

            bool contains(Point3 const& p) const
            {
                if (isNull()) return false;
                if (isInfinite()) return true;

                return
                    min_point.x <= p.x && p.x <= max_point.x &&
                    min_point.y <= p.y && p.y <= max_point.y &&
                    min_point.z <= p.z && p.z <= max_point.z;
            }

            bool contains(BoundingBox const& a) const
            {
                if (a.isNull() || isInfinite()) return true;
                if (isNull() || a.isInfinite()) return false;

                return
                    min_point.x <= a.min_point.x &&
                    min_point.y <= a.min_point.y &&
                    min_point.z <= a.min_point.z &&
                    a.max_point.x <= max_point.x &&
                    a.max_point.y <= max_point.y &&
                    a.max_point.z <= max_point.z;
            }

            BoundingBox intersection(BoundingBox const& a) const
            {
                if (isNull() || a.isNull()) return {};
                if (isInfinite()) return a;
                if (a.isInfinite()) return *this;

                auto inter_min_point = max(min_point, a.min_point);
                auto inter_max_point = min(max_point, a.max_point);

                if (inter_min_point.x >= inter_max_point.x) return {};
                if (inter_min_point.y >= inter_max_point.y) return {};
                if (inter_min_point.z >= inter_max_point.z) return {};

                return {inter_min_point, inter_max_point};
            }

            bool intersect(BoundingBox const& a) const
            {
                if (isNull() || a.isNull()) return false;
                if (isInfinite() || a.isInfinite()) return true;

                bool x = (max_point.x >= a.min_point.x) && (min_point.x <= a.max_point.x);
                bool y = (max_point.y >= a.min_point.y) && (min_point.y <= a.max_point.y);
                bool z = (max_point.z >= a.min_point.z) && (min_point.z <= a.max_point.z);
                return (x && y && z);
            }

            bool intersect(Ray const& ray, float & t0, float & t1) const;
            bool intersect(Sphere const& sphere) const;
            bool intersect(Plane const& plane) const;

            void merge(BoundingBox const& a)
            {
                if (isInfinite() || a.isNull()) return;
                if (a.isInfinite()) { setInfinite(); return; }
                if (isNull()) { *this = a; return; }

                max_point = max(max_point, a.max_point);
                min_point = min(min_point, a.min_point);
            }

            void merge(Point3 const& p)
            {
                switch(extent)
                {
                    case Extent::null: setExtents(p, p); break;
                    case Extent::infinite: break;
                    case Extent::finite: setExtents(max(max_point, p), min(min_point, p)); break;
                }
                SS_UTIL_UNREACHABLE();
            }

            [[deprecated("Use a = BoundingBox{p1, p2} instead of a.fromPoints(p1, p2).")]]
            BoundingBox & fromPoints(Point3 const& p1, Point3 const& p2) { return (*this = BoundingBox{p1, p2}); }

            void transformAffine(Matrix4 const& m)
            {
                m.ensure_affine();
                if (extent != Extent::finite) return;

                Point3 center = m * getCenter();
                Vector3 half = abs(Matrix3{m}) * getHalfSize();

                setExtents(center + half, center - half);
            }
        };

        inline bool operator != (BoundingBox const& a, BoundingBox const& b) { return !(a == b); }
        inline bool operator == (BoundingBox const& a, BoundingBox const& b)
        {
            if (a.extent != b.extent) return false;
            switch (a.extent) {
                case BoundingBox::Extent::null: return true;
                case BoundingBox::Extent::infinite: return true;
                case BoundingBox::Extent::finite:
                    return
                        a.min_point == b.min_point &&
                        a.max_point == b.max_point;
            }
            SS_UTIL_UNREACHABLE();
        }

        inline std::ostream & operator << (std::ostream & o, BoundingBox const& a)
        {
            return (o << "BoundingBox{\n"
                << "    .max_point = " << a.max_point << ",\n"
                << "    .min_point = " << a.min_point << ",\n"
                << "}; // size = " << a.getSize());
        }
    }
}

#include "../util/unreachable.undef.hpp"

