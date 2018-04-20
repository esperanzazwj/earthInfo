#pragma once
#include "../lib/glm/mat3.hpp"
#include "../lib/glm/op/common.hpp"
#include "../lib/glm/op/mat.hpp"
#include "forward.hpp"
#include "vector3.hpp"
#include <iostream>

namespace ss
{
    namespace math
    {
        struct Matrix3
        {
            glm::mat3 m;

            Matrix3() = default; // identity
            explicit Matrix3(float x) : m{x} {}
            explicit Matrix3(Vector3 a) : Matrix3{a[0], a[1], a[2]} {}
            explicit Matrix3(Vector3 x, Vector3 y, Vector3 z) : m{x.v, y.v, z.v} {}
            explicit Matrix3(float a, float b, float c)
                : m{
                    glm::vec3{a, 0.0f, 0.0f},
                    glm::vec3{0.0f, b, 0.0f},
                    glm::vec3{0.0f, 0.0f, c},
                }
            {}

            explicit Matrix3(glm::mat3 x) : m{x} {}

            [[deprecated]] Matrix3(float (&xs)[9]) : m{glm::make_mat3(xs)} {}
            [[deprecated]] Matrix3(float xs[]) : m{glm::make_mat3(xs)} {}

            void swap(Matrix3 & a) { std::swap(m, a.m); }

            auto& operator [] (int i) const { return m[i]; }
            auto& operator [] (int i)       { return m[i]; }

            auto ptr() const { return &m[0][0]; }
            auto ptr()       { return &m[0][0]; }

            auto operator + () const { return Matrix3{+m}; }
            auto operator - () const { return Matrix3{-m}; }

            auto& operator += (Matrix3 const& a) { m += a.m; return *this; }
            auto& operator -= (Matrix3 const& a) { m -= a.m; return *this; }
            auto& operator *= (Matrix3 const& a) { m *= a.m; return *this; }
            auto& operator /= (Matrix3 const& a) { m /= a.m; return *this; }

            [[deprecated(
                "\n\n"
                "Clever use of multiplication order can get rid of the transpose."
                "\n\n"
                "Set `transpose = GL_TRUE` when calling glUniformMatrix* to transpose "
                "when uploading to VRAM."
                "\n\n"
            )]]
            auto transpose() const { return Matrix3{glm::transpose(m)}; }

            // undefined behavior if the matrix is not orthogonal
            auto inverse_as_if_orthogonal() const { return Matrix3{glm::transpose(m)}; }
            auto inverse() const { return Matrix3{glm::inverse(m)}; }

            auto determinant() const { return glm::determinant(m); }
            auto orthonormalize() const { return Matrix3{glm::orthonormalize(m)}; }
        };

        inline bool operator == (Matrix3 const& a, Matrix3 const& b) { return a.m == b.m; }
        inline bool operator != (Matrix3 const& a, Matrix3 const& b) { return a.m != b.m; }

        inline Matrix3 operator + (Matrix3 const& a, Matrix3 const& b) { return Matrix3{a.m + b.m}; }
        inline Matrix3 operator - (Matrix3 const& a, Matrix3 const& b) { return Matrix3{a.m - b.m}; }
        inline Matrix3 operator * (Matrix3 const& a, Matrix3 const& b) { return Matrix3{a.m * b.m}; }
        inline Vector3 operator * (Matrix3 const& a, Vector3 const& b) { return Vector3{a.m * b.v}; }
        inline Vector3 operator * (Vector3 const& a, Matrix3 const& b) { return Vector3{a.v * b.m}; }

        inline Matrix3 abs(Matrix3 const& a)
        {
            return Matrix3{
                Vector3{abs(a.m[0])},
                Vector3{abs(a.m[1])},
                Vector3{abs(a.m[2])},
            };
        }

        inline std::ostream & operator << (std::ostream & o, Matrix3 const& m)
        {
            return (o << "Matrix3{\n"
                << "    " << Vector3{m[0]} << ",\n"
                << "    " << Vector3{m[1]} << ",\n"
                << "    " << Vector3{m[2]} << ",\n"
                << "}");
        }
    }
}

