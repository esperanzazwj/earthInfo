#pragma once
#include "../lib/glm/quat.hpp"
#include "forward.hpp"
#include "vector3.hpp"
#include "matrix3.hpp"
#include <iostream>

namespace ss
{
    namespace math
    {
        bool operator == (Quaternion const& a, Quaternion const& b);
        bool operator != (Quaternion const& a, Quaternion const& b);

        Quaternion operator + (Quaternion const& a, Quaternion const& b);
        Quaternion operator * (Quaternion const& a, Quaternion const& b);
        Quaternion operator * (Quaternion const& a, float b);
        Quaternion operator * (float a, Quaternion const& b);

        float dot(Quaternion const& a, Quaternion const& b);

        struct Quaternion
        {
            union {
                struct {
                    float x;
                    float y;
                    float z;
                    float w;
                };
                glm::quat q;    // weirdly stored as x, y, z, w
            };

            Quaternion() : q{} {}     // identity
            Quaternion(float w, float x, float y, float z) : q{w, x, y, z} {}

            explicit Quaternion(float angle, Vector3 const& axis) : q{angleAxis(angle, axis.v)} {}

            explicit Quaternion(Matrix3 a) : q{a.m} {}
            explicit operator Matrix3 () const { return Matrix3{mat3_cast(q)}; }

            explicit Quaternion(glm::quat x) : q{x} {}

            void swap(Quaternion & a) { std::swap(q, a.q); }

            [[deprecated("DEBUG")]] auto& operator [] (int i) const { return q[i]; }
            [[deprecated("DEBUG")]] auto& operator [] (int i)       { return q[i]; }

            [[deprecated("DEBUG")]] auto ptr() const { return &q[0]; }
            [[deprecated("DEBUG")]] auto ptr()       { return &q[0]; }

            auto operator + () const { return Quaternion{+q}; }
            auto operator - () const { return Quaternion{-q}; }

            auto& operator += (Quaternion const& a) { q += a.q; return *this; }
            auto& operator -= (Quaternion const& a) { q -= a.q; return *this; }
            auto& operator *= (Quaternion const& a) { q *= a.q; return *this; }
            auto& operator *= (float a) { q *= a; return *this; }

            float dotProduct(Quaternion const& a) const { return dot(*this, a); }
            auto inverse() const { return Quaternion{glm::inverse(q)}; }

            // special values
            [[deprecated("Use Quaternion{0.0f, 0.0f, 0.0f, 0.0f}. Why do you need zero quaternions?")]]
            static const Quaternion ZERO;
            [[deprecated("Use Quaternion{} for identity quaternions")]]
            static const Quaternion IDENTITY;
        };

        inline bool operator == (Quaternion const& a, Quaternion const& b) { return a.q == b.q; }
        inline bool operator != (Quaternion const& a, Quaternion const& b) { return a.q != b.q; }

        inline Quaternion operator + (Quaternion const& a, Quaternion const& b) { return Quaternion{a.q + b.q}; }
        inline Quaternion operator * (Quaternion const& a, Quaternion const& b) { return Quaternion{a.q * b.q}; }
        inline Quaternion operator * (Quaternion const& a, float b) { return Quaternion{a.q * b}; }
        inline Quaternion operator * (float a, Quaternion const& b) { return Quaternion{a * b.q}; }
        inline Vector3 operator * (Quaternion const& a, Vector3 const& b) { return Vector3{a.q * b.v}; }

        inline float dot(Quaternion const& a, Quaternion const& b) { return dot(a.q, b.q); }

        inline std::ostream & operator << (std::ostream & o, Quaternion const& a)
        {
            return (o << "Quaternion{w=" << a.w << ", " << a.x << ", " << a.y << ", " << a.z << "}");
        }
    }
}

