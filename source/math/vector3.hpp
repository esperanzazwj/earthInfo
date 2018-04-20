#pragma once
#include "../lib/glm/vec3.hpp"
#include "../lib/glm/relational.hpp"
#include "../lib/glm/op/geom.hpp"
#include "../lib/glm/op/trig.hpp"
#include "../lib/glm/op/common.hpp"
#include "forward.hpp"
#include <iostream>
#include <string>
#include <cmath>

namespace ss
{
    namespace math
    {
        bool operator == (Vector3 const& a, Vector3 const& b);
        bool operator != (Vector3 const& a, Vector3 const& b);

        Vector3 operator + (Vector3 const& a, Vector3 const& b);
        Vector3 operator - (Vector3 const& a, Vector3 const& b);
        Vector3 operator * (Vector3 const& a, Vector3 const& b);
        Vector3 operator / (Vector3 const& a, Vector3 const& b);

        bool operator < (Vector3 const& a, Vector3 const& b);
        bool operator > (Vector3 const& a, Vector3 const& b);

        float dot(Vector3 const& a, Vector3 const& b);
        float abs_dot(Vector3 const& a, Vector3 const& b);
        Vector3 mix(Vector3 const& a, Vector3 const& b, float t);
        Vector3 min(Vector3 const& a, Vector3 const& b);
        Vector3 max(Vector3 const& a, Vector3 const& b);
        float min(Vector3 const& a);
        float max(Vector3 const& a);
        void swap_unless_min_max(Vector3 & a, Vector3 & b);

        struct Vector3
        {
            union {
                struct {
                    float x;
                    float y;
                    float z;
                };
                glm::vec3 v;
            };

            Vector3() : v{} {}
            Vector3(float x) : v{x} {}
            Vector3(float x, float y, float z) : v{x, y, z} {}

            explicit Vector3(glm::vec3 x) : v{x} {}

            Vector3(float (&xs)[3]) : v{xs[0], xs[1], xs[2]} {}
            Vector3(float xs[])     : v{xs[0], xs[1], xs[2]} {}

            Vector3(int (&xs)[3])
                : v{
                    float(xs[0]),
                    float(xs[1]),
                    float(xs[2]),
                }
            {}

            Vector3(int xs[])
                : v{
                    float(xs[0]),
                    float(xs[1]),
                    float(xs[2]),
                }
            {}

            explicit Vector3(std::string const& xs);                // extract values from a string similar to "x,y,z"

            void swap(Vector3 & a) { std::swap(v, a.v); }

            auto& operator [] (int i) const { return v[i]; }
            auto& operator [] (int i)       { return v[i]; }

            auto ptr() const { return &v[0]; }
            auto ptr()       { return &v[0]; }

            auto operator + () const { return Vector3{+v}; }
            auto operator - () const { return Vector3{-v}; }

            auto& operator += (Vector3 const& a) { v += a.v; return *this; }
            auto& operator -= (Vector3 const& a) { v -= a.v; return *this; }
            auto& operator *= (Vector3 const& a) { v *= a.v; return *this; }
            auto& operator /= (Vector3 const& a) { v /= a.v; return *this; }

            float distance(Vector3 const& a) { return (*this - a).length(); }
            float distSquared(Vector3 const& a) { return (*this - a).squaredLength(); }
            float dotProduct(Vector3 const& a) const { return dot(*this, a); }
            float absDotProduct(Vector3 const& a) const { return abs_dot(*this, a); }

            Vector3 midPoint(Vector3 const& a) const { return mix(*this, a, 0.5f); }
            float squaredLength() const { return dot(v, v); }
            float length() const { return glm::length(v); }
            Vector3 hat() const { return Vector3{glm::normalize(v)}; }

            float normalize()
            {
                float flen = length();
                if (flen > 1e-6f) v = glm::normalize(v);
                return flen;
            }

            [[deprecated("Use a = min(a, b) instead of a.makeFloor(b).")]]
            void makeFloor(Vector3 const& a) { *this = min(*this, a); }

            [[deprecated("Use a = max(a, b) instead of a.makeCeil(b).")]]
            void makeCeil(Vector3 const& a) { *this = max(*this, a); }

            Vector3 perpendicular() const
            {
                constexpr auto fSquareZero = 1e-06f * 1e-06f;

                Vector3 perp = crossProduct(Vector3::UNIT_X);
                if (perp.squaredLength() < fSquareZero)
                    perp = crossProduct(Vector3::UNIT_Y);

                return perp.hat();
            }

            float angleBetween(Vector3 const& a) const
            {
                float lenProduct = glm::max(length() * a.length(), 1e-6f);
                float f = glm::clamp(dotProduct(a) / lenProduct, -1.0f, 1.0f);
                return std::acos(f);
            }

            bool isZeroLength() const
            {
                float sqlen = squaredLength();
                return (sqlen < (1e-06f * 1e-06f));
            }

            Vector3 crossProduct(Vector3 const& a) const
            {
                return Vector3{
                    y * a.z - z * a.y,
                    z * a.x - x * a.z,
                    x * a.y - y * a.x,
                };
            }

            Vector3 reflect(Vector3 const& normal) const
            {
                return Vector3{glm::reflect(v, normal.v)};
            }

            bool positionEquals(Vector3 const& a, float tolerance = 1e-03f) const
            {
                return all(lessThanEqual(abs(v - a.v), glm::vec3{tolerance}));
            }

            bool directionEquals(Vector3 const& a, float tolerance) const
            {
                float d = dotProduct(a);
                float angle = std::abs(std::acos(d));
                return (angle <= tolerance);
            }

            static const Vector3 ZERO;
            static const Vector3 UNIT_SCALE;
            static const Vector3 UNIT_X;
            static const Vector3 UNIT_Y;
            static const Vector3 UNIT_Z;
            static const Vector3 NEGATIVE_UNIT_X;
            static const Vector3 NEGATIVE_UNIT_Y;
            static const Vector3 NEGATIVE_UNIT_Z;
        };


        inline bool operator == (Vector3 const& a, Vector3 const& b) { return a.v == b.v; }
        inline bool operator != (Vector3 const& a, Vector3 const& b) { return a.v != b.v; }

        inline Vector3 operator + (Vector3 const& a, Vector3 const& b) { return Vector3{a.v + b.v}; }
        inline Vector3 operator - (Vector3 const& a, Vector3 const& b) { return Vector3{a.v - b.v}; }
        inline Vector3 operator * (Vector3 const& a, Vector3 const& b) { return Vector3{a.v * b.v}; }
        inline Vector3 operator / (Vector3 const& a, Vector3 const& b) { return Vector3{a.v / b.v}; }

        inline bool operator < (Vector3 const& a, Vector3 const& b) { return all(   lessThan(a.v, b.v)); }
        inline bool operator > (Vector3 const& a, Vector3 const& b) { return all(greaterThan(a.v, b.v)); }

        inline float dot(Vector3 const& a, Vector3 const& b) { return dot(a.v, b.v); }
        inline Vector3 mix(Vector3 const& a, Vector3 const& b, float t) { return Vector3{mix(a.v, b.v, t)}; }
        inline Vector3 min(Vector3 const& a, Vector3 const& b) { return Vector3{min(a.v, b.v)}; }
        inline Vector3 max(Vector3 const& a, Vector3 const& b) { return Vector3{max(a.v, b.v)}; }
        inline float min(Vector3 const& a) { return compMin(a.v); }
        inline float max(Vector3 const& a) { return compMax(a.v); }

        inline void swap_unless_min_max(Vector3 & a, Vector3 & b)
        {
            auto min_vec = min(a, b);
            auto max_vec = max(a, b);
            a = min_vec;
            b = max_vec;
        }

        inline float abs_dot(Vector3 const& a, Vector3 const& b)
        {
            return std::abs(a.x * b.x) + std::abs(a.y * b.y) + std::abs(a.z * b.z);
        }

        inline std::ostream & operator << (std::ostream & o, Vector3 const& a)
        {
            return (o << "Vector3{" << a.x << ", " << a.y << ", " << a.z << "}");
        }
    }
}

