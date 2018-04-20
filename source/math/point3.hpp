#pragma once
#include "../lib/glm/vec3.hpp"
#include "../lib/glm/relational.hpp"
#include "../lib/glm/op/geom.hpp"
#include "../lib/glm/op/trig.hpp"
#include "../lib/glm/op/common.hpp"
#include "vector3.hpp"
#include "forward.hpp"
#include <iostream>

namespace ss
{
    namespace math
    {
        bool operator == (Point3 const& a, Point3 const& b);
        bool operator != (Point3 const& a, Point3 const& b);

        Point3 operator + (Point3 const& a, Point3 const& b);
        Point3 operator + (Point3 const& a, Vector3 const& b);
        Vector3 operator - (Point3 const& a, Point3 const& b);
        Point3 operator - (Point3 const& a, Vector3 const& b);
        Point3 operator * (Point3 const& a, Point3 const& b);
        Point3 operator / (Point3 const& a, Point3 const& b);

        struct Point3 {
            union {
                struct {
                    float x;
                    float y;
                    float z;
                };
                glm::vec3 v;
            };

            Point3() : v{} {}
            Point3(float x) : v{x} {}
            Point3(float x, float y, float z) : v{x, y, z} {}

            Point3(Vector3 a) : v{a.v} {}
            operator Vector3 () const { return Vector3{v}; }

            explicit Point3(glm::vec3 x) : v{x} {}

            auto& operator [] (int i) const { return v[i]; }
            auto& operator [] (int i)       { return v[i]; }

            auto ptr() const { return &v[0]; }
            auto ptr()       { return &v[0]; }

            auto operator + () const { return Point3{+v}; }
            auto operator - () const { return Point3{-v}; }

            auto& operator += (Point3 const& a) { v += a.v; return *this; }
            auto& operator += (Vector3 const& a) { v += a.v; return *this; }
            auto& operator -= (Vector3 const& a) { v -= a.v; return *this; }
            auto& operator *= (Point3 const& a) { v *= a.v; return *this; }
            auto& operator /= (Point3 const& a) { v /= a.v; return *this; }

            float distance(Point3 const& a) { return (*this - a).length(); }
            float distSquared(Point3 const& a) { return (*this - a).squaredLength(); }

            float squaredLength() const { return dot(v, v); }
            float length() const { return glm::length(v); }
        };

        inline bool operator != (Point3 const& a, Point3 const& b) { return !(a == b); }
        inline bool operator == (Point3 const& a, Point3 const& b)
        {
            constexpr auto tolerance = 1e-6f;
            return all(lessThanEqual(abs(a.v - b.v), glm::vec3{tolerance}));
        }

        inline Point3 operator + (Point3 const& a, Point3 const& b) { return Point3{a.v + b.v}; }
        inline Point3 operator + (Point3 const& a, Vector3 const& b) { return Point3{a.v + b.v}; }
        inline Vector3 operator - (Point3 const& a, Point3 const& b) { return Vector3{a.v - b.v}; }
        inline Point3 operator - (Point3 const& a, Vector3 const& b) { return Point3{a.v - b.v}; }
        inline Point3 operator * (Point3 const& a, Point3 const& b) { return Point3{a.v * b.v}; }
        inline Point3 operator / (Point3 const& a, Point3 const& b) { return Point3{a.v / b.v}; }

        inline Point3 min(Point3 const& a, Point3 const& b) { return Point3{glm::min(a.v, b.v)}; }
        inline Point3 max(Point3 const& a, Point3 const& b) { return Point3{glm::max(a.v, b.v)}; }
        inline Point3 mix(Point3 const& a, Point3 const& b, float x) { return Point3{glm::mix(a.v, b.v, x)}; }

        inline std::ostream & operator << (std::ostream & o, Point3 const& a)
        {
            return (o << "Point3{" << a.x << ", " << a.y << ", " << a.z << "}");
        }
    }
}

