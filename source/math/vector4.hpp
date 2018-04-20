#pragma once
#include "../lib/glm/vec4.hpp"
#include "../lib/glm/op/common.hpp"
#include "../lib/glm/op/geom.hpp"
#include "vector3.hpp"
#include "forward.hpp"
#include <iostream>

namespace ss
{
    namespace math
    {
        float dot(Vector4 const& a, Vector4 const& b);
        float min(Vector4 const& a);
        float max(Vector4 const& a);

        struct Vector4
        {
            union {
                struct {
                    float x;
                    float y;
                    float z;
                    float w;
                };
                glm::vec4 v;
            };

            Vector4() : v{} {}
            Vector4(float x) : v{x} {}
            Vector4(float x, float y, float z, float w) : v{x, y, z, w} {}

            explicit Vector4(Vector3 a, float w) : v{a.v, w} {}
            explicit Vector4(glm::vec4 x) : v{x} {}
            explicit operator Vector3 () { return Vector3{glm::vec3{v}}; }

            Vector4(float (&xs)[4]) : v{xs[0], xs[1], xs[2], xs[3]} {}
            Vector4(float xs[])     : v{xs[0], xs[1], xs[2], xs[3]} {}

            Vector4(int (&xs)[4])
                : v{
                    float(xs[0]),
                    float(xs[1]),
                    float(xs[2]),
                    float(xs[3]),
                }
            {}

            Vector4(int xs[])
                : v{
                    float(xs[0]),
                    float(xs[1]),
                    float(xs[2]),
                    float(xs[3]),
                }
            {}

            void swap(Vector4 & a) { std::swap(v, a.v); }

            auto& operator [] (int i) const { return v[i]; }
            auto& operator [] (int i)       { return v[i]; }

            auto ptr() const { return &v[0]; }
            auto ptr()       { return &v[0]; }

            auto operator + () const { return Vector4{+v}; }
            auto operator - () const { return Vector4{-v}; }

            auto& operator += (Vector4 const& a) { v += a.v; return *this; }
            auto& operator -= (Vector4 const& a) { v -= a.v; return *this; }
            auto& operator *= (Vector4 const& a) { v *= a.v; return *this; }
            auto& operator /= (Vector4 const& a) { v /= a.v; return *this; }

            float dotProduct(Vector4 const& a) const { return dot(*this, a); }

            static const Vector4 ZERO;
        };

        inline bool operator == (Vector4 const& a, Vector4 const& b) { return a.v == b.v; }
        inline bool operator != (Vector4 const& a, Vector4 const& b) { return a.v != b.v; }

        inline Vector4 operator + (Vector4 const& a, Vector4 const& b) { return Vector4{a.v + b.v}; }
        inline Vector4 operator - (Vector4 const& a, Vector4 const& b) { return Vector4{a.v - b.v}; }
        inline Vector4 operator * (Vector4 const& a, Vector4 const& b) { return Vector4{a.v * b.v}; }
        inline Vector4 operator / (Vector4 const& a, Vector4 const& b) { return Vector4{a.v / b.v}; }

        inline float dot(Vector4 const& a, Vector4 const& b) { return dot(a.v, b.v); }
        inline float min(Vector4 const& a) { return compMin(a.v); }
        inline float max(Vector4 const& a) { return compMax(a.v); }

        inline std::ostream & operator << (std::ostream & o, Vector4 const& a)
        {
            return (o << "Vector4{" << a.x << ", " << a.y << ", " << a.z << ", " << a.w << "}");
        }
    }
}

