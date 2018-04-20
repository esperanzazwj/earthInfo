#pragma once
#include "../lib/glm/vec2.hpp"
#include "../lib/glm/relational.hpp"
#include "../lib/glm/op/geom.hpp"
#include "../lib/glm/op/trig.hpp"
#include "../lib/glm/op/common.hpp"
#include "forward.hpp"
#include <iostream>

namespace ss
{
    namespace math
    {
        bool operator == (Vector2 const& a, Vector2 const& b);
        bool operator != (Vector2 const& a, Vector2 const& b);

        Vector2 operator + (Vector2 const& a, Vector2 const& b);
        Vector2 operator - (Vector2 const& a, Vector2 const& b);
        Vector2 operator * (Vector2 const& a, Vector2 const& b);
        Vector2 operator / (Vector2 const& a, Vector2 const& b);

        bool operator < (Vector2 const& a, Vector2 const& b);
        bool operator > (Vector2 const& a, Vector2 const& b);

        float dot(Vector2 const& a, Vector2 const& b);
        Vector2 mix(Vector2 const& a, Vector2 const& b, float t);
        Vector2 min(Vector2 const& a, Vector2 const& b);
        Vector2 max(Vector2 const& a, Vector2 const& b);

        struct Vector2
        {
            union {
                struct {
                    float x;
                    float y;
                };
                glm::vec2 v;
            };

            Vector2() : v{} {}
            Vector2(float x) : v{x} {}
            Vector2(float x, float y) : v{x, y} {}

            explicit Vector2(glm::vec2 x) : v{x} {}

            Vector2(float (&xs)[2]) : v{xs[0], xs[1]} {}
            Vector2(float xs[])     : v{xs[0], xs[1]} {}

            Vector2(int (&xs)[2])
                : v{
                    float(xs[0]),
                    float(xs[1]),
                }
            {}

            Vector2(int xs[])
                : v{
                    float(xs[0]),
                    float(xs[1]),
                }
            {}

            void swap(Vector2 & a)
            {
                std::swap(v, a.v);
            }

            auto& operator [] (int i) const { return v[i]; }
            auto& operator [] (int i)       { return v[i]; }

            auto ptr() const { return &v[0]; }
            auto ptr()       { return &v[0]; }

            auto operator + () const { return Vector2{+v}; }
            auto operator - () const { return Vector2{-v}; }

            auto& operator += (Vector2 const& a) { v += a.v; return *this; }
            auto& operator -= (Vector2 const& a) { v -= a.v; return *this; }
            auto& operator *= (Vector2 const& a) { v *= a.v; return *this; }
            auto& operator /= (Vector2 const& a) { v /= a.v; return *this; }

            float distance(Vector2 const& a) { return (*this - a).length(); }
            float distSquared(Vector2 const& a) { return (*this - a).squaredLength(); }
            float dotProduct(Vector2 const& a) const { return dot(*this, a); }

            Vector2 midPoint(Vector2 const& a) const { return mix(*this, a, 0.5f); }
            float squaredLength() const { return dot(v, v); }
            float length() const { return glm::length(v); }
            Vector2 hat() const { return Vector2{glm::normalize(v)}; }

            float normalize()
            {
                float flen = length();
                if (flen > 1e-6f) v = glm::normalize(v);
                return flen;
            }

            [[deprecated("Use a = min(a, b) instead of a.makeFloor(b).")]]
            void makeFloor(Vector2 const& a) { *this = min(*this, a); }

            [[deprecated("Use a = max(a, b) instead of a.makeCeil(b).")]]
            void makeCeil(Vector2 const& a) { *this = max(*this, a); }

            Vector2 perpendicular() const
            {
                return Vector2{-y, x};
            }

            bool isZeroLength() const
            {
                float sqlen = squaredLength();
                return (sqlen < (1e-06f * 1e-06f));
            }

            float crossProduct(Vector2 const& a) const
            {
                return x * a.y - y * a.x;
            }

            Vector2 reflect(Vector2 const& normal) const
            {
                return Vector2{glm::reflect(v, normal.v)};
            }

            static const Vector2 ZERO;
            static const Vector2 UNIT_SCALE;
            static const Vector2 UNIT_X;
            static const Vector2 UNIT_Y;
            static const Vector2 NEGATIVE_UNIT_X;
            static const Vector2 NEGATIVE_UNIT_Y;
        };

        inline bool operator == (Vector2 const& a, Vector2 const& b) { return a.v == b.v; }
        inline bool operator != (Vector2 const& a, Vector2 const& b) { return a.v != b.v; }

        inline Vector2 operator + (Vector2 const& a, Vector2 const& b) { return Vector2{a.v + b.v}; }
        inline Vector2 operator - (Vector2 const& a, Vector2 const& b) { return Vector2{a.v - b.v}; }
        inline Vector2 operator * (Vector2 const& a, Vector2 const& b) { return Vector2{a.v * b.v}; }
        inline Vector2 operator / (Vector2 const& a, Vector2 const& b) { return Vector2{a.v / b.v}; }

        inline bool operator < (Vector2 const& a, Vector2 const& b) { return all(   lessThan(a.v, b.v)); }
        inline bool operator > (Vector2 const& a, Vector2 const& b) { return all(greaterThan(a.v, b.v)); }

        inline float dot(Vector2 const& a, Vector2 const& b) { return dot(a.v, b.v); }
        inline Vector2 mix(Vector2 const& a, Vector2 const& b, float t) { return Vector2{mix(a.v, b.v, t)}; }
        inline Vector2 min(Vector2 const& a, Vector2 const& b) { return Vector2{min(a.v, b.v)}; }
        inline Vector2 max(Vector2 const& a, Vector2 const& b) { return Vector2{max(a.v, b.v)}; }

        inline std::ostream & operator << (std::ostream & o, Vector2 const& a)
        {
            return (o << "Vector2{" << a.x << ", " << a.y <<  "}");
        }
    }
}

