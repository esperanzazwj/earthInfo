#pragma once
#include "../lib/glm/mat4.hpp"
#include "../lib/glm/op/common.hpp"
#include "../lib/glm/op/mat.hpp"
#include "forward.hpp"
#include "vector3.hpp"
#include "vector4.hpp"
#include "matrix3.hpp"
#include "point3.hpp"
#include "plane.hpp"
#include "quaternion.hpp"
#include <iostream>
#include <stdexcept>

namespace ss
{
    namespace math
    {
        bool operator == (Matrix4 const& a, Matrix4 const& b);
        bool operator != (Matrix4 const& a, Matrix4 const& b);

        Matrix4 operator + (Matrix4 const& a, Matrix4 const& b);
        Matrix4 operator - (Matrix4 const& a, Matrix4 const& b);
        Matrix4 operator * (Matrix4 const& a, Matrix4 const& b);
        Vector4 operator * (Matrix4 const& a, Vector4 const& b);
        Vector4 operator * (Vector4 const& a, Matrix4 const& b);
        Matrix4 operator * (Matrix4 const& a, float b);
        Matrix4 operator * (float a, Matrix4 const& b);
        Point3 operator * (Matrix4 const& a, Point3 const& b);
        Plane operator * (Matrix4 const& a, Plane const& b);

        struct Matrix4
        {
            glm::mat4 m;

            Matrix4() = default;    // identity
            Matrix4(Matrix3 a) : m{a.m} {}
            explicit Matrix4(float x) : m{x} {}
            explicit Matrix4(Vector3 a) : Matrix4{a[0], a[1], a[2]} {}
            explicit Matrix4(Vector4 a) : Matrix4{a[0], a[1], a[2], a[3]} {}
            explicit Matrix4(Vector4 x, Vector4 y, Vector4 z, Vector4 w)
                : m{x.v, y.v, z.v, w.v} {}

            explicit Matrix4(float a, float b, float c, float d=1.0f)
                : m{
                    glm::vec4{a, 0.0f, 0.0f, 0.0f},
                    glm::vec4{0.0f, b, 0.0f, 0.0f},
                    glm::vec4{0.0f, 0.0f, c, 0.0f},
                    glm::vec4{0.0f, 0.0f, 0.0f, d},
                }
            {}
            explicit Matrix4(Vector3 x, Vector3 y, Vector3 z)
                : Matrix4{Matrix3{x, y, z}}
            {}
            explicit Matrix4(Vector3 x, Vector3 y, Vector3 z, Vector3 pos)
                : Matrix4{
                    Matrix3{x, y, z},
                    pos,
                }
            {}
            explicit Matrix4(Matrix3 a, Vector3 pos)
                : m{
                    glm::vec4{a[0], 0.0f},
                    glm::vec4{a[1], 0.0f},
                    glm::vec4{a[2], 0.0f},
                    glm::vec4{pos.v, 1.0f},
                }
            {}
            explicit Matrix4(Quaternion const& rot, Vector3 scale, Vector3 pos)
                : Matrix4{static_cast<Matrix3>(rot) * Matrix3{scale}, pos}
            {}

            explicit Matrix4(glm::mat4 x) : m{x} {}
            explicit operator Matrix3 () const { return Matrix3{glm::mat3{m}}; }

            [[deprecated]] Matrix4(float (&xs)[16]) : m{glm::make_mat4(xs)} {}
            [[deprecated]] Matrix4(float xs[]) : m{glm::make_mat4(xs)} {}

            void swap(Matrix4 & a) { std::swap(m, a.m); }

            auto& operator [] (int i) const { return m[i]; }
            auto& operator [] (int i)       { return m[i]; }

            auto ptr() const { return &m[0][0]; }
            auto ptr()       { return &m[0][0]; }

            auto operator + () const { return Matrix4{+m}; }
            auto operator - () const { return Matrix4{-m}; }

            auto& operator += (Matrix4 const& a) { m += a.m; return *this; }
            auto& operator -= (Matrix4 const& a) { m -= a.m; return *this; }
            auto& operator *= (Matrix4 const& a) { m *= a.m; return *this; }
            auto& operator /= (Matrix4 const& a) { m /= a.m; return *this; }

            [[deprecated(
                "\n\n"
                "Clever use of multiplication order can get rid of the transpose."
                "\n\n"
                "Set `transpose = GL_TRUE` when calling glUniformMatrix* to transpose "
                "when uploading to VRAM."
                "\n\n"
            )]]
            auto transpose() const { return Matrix4{glm::transpose(m)}; }
            auto inverse() const { return Matrix4{glm::inverse(m)}; }
            auto determinant() const { return glm::determinant(m); }

            [[deprecated("Use res = Matrix3{m} to replace m.extract3x3Matrix(res).")]]
            void extract3x3Matrix(Matrix3 & result) const { result = Matrix3{*this}; }

            [[deprecated("Use matrix multiplication")]]
            Matrix4 concatenate(Matrix4 const& m2) const
            {
                return *this * m2;
            }

            void setTrans(Vector3 const& a) { m[3] = glm::vec4{a.v, m[3][3]}; }
            Vector3 getTrans() const { return Vector3{glm::vec3{m[3]}}; }

            Matrix4 getRotate() const
            {
                return Matrix4{
                    Vector4{m[0]},
                    Vector4{m[1]},
                    Vector4{m[2]},
                    Vector4{0.0f, 0.0f, 0.0f, m[3][3]},
                };
            }

            void setScale(Vector3 const& a)
            {
                m[0][0] = a[0];
                m[1][1] = a[1];
                m[2][2] = a[2];
            }

            [[deprecated("Use Matrix4{orientation, scale, position} instead.")]]
            void makeTransform(Vector3 const& position, Vector3 const& scale, Quaternion const& orientation)
            {
                *this = Matrix4{orientation, scale, position};
            }

            void ensure_affine() const
            {
                if (m[0][3] != 0.0f || m[1][3] != 0.0f || m[2][3] != 0.0f || m[3][3] != 1.0f)
                    throw std::logic_error{"Must be an affine matrix!"};
            }

            [[deprecated("Use normal matrix multiplication")]]
            Vector3 transformAffine(Vector3 const& a) const
            {
                ensure_affine();
                return Vector3{glm::vec3{m * glm::vec4{a.v, 1.0f}}};
            }

            [[deprecated("Use normal matrix multiplication")]]
            Vector4 transformAffine(Vector4 const& a) const
            {
                ensure_affine();
                return *this * a;
            }

            [[deprecated("Use Matrix4{0.0f} for zero matrix")]]
            static const Matrix4 ZERO;
            [[deprecated("Use Matrix4{} for identity matrix")]]
            static const Matrix4 IDENTITY;
        };

        inline bool operator == (Matrix4 const& a, Matrix4 const& b) { return a.m == b.m; }
        inline bool operator != (Matrix4 const& a, Matrix4 const& b) { return a.m != b.m; }

        inline Matrix4 operator + (Matrix4 const& a, Matrix4 const& b) { return Matrix4{a.m + b.m}; }
        inline Matrix4 operator - (Matrix4 const& a, Matrix4 const& b) { return Matrix4{a.m - b.m}; }
        inline Matrix4 operator * (Matrix4 const& a, Matrix4 const& b) { return Matrix4{a.m * b.m}; }
        inline Vector4 operator * (Matrix4 const& a, Vector4 const& b) { return Vector4{a.m * b.v}; }
        inline Vector4 operator * (Vector4 const& a, Matrix4 const& b) { return Vector4{a.v * b.m}; }
        inline Matrix4 operator * (Matrix4 const& a, float b) { return Matrix4{a.m * b}; }
        inline Matrix4 operator * (float a, Matrix4 const& b) { return Matrix4{a * b.m}; }

        inline Point3 operator * (Matrix4 const& a, Point3 const& b)
        {
            auto c = a.m * glm::vec4{b.v, 1.0f};
            return Point3{glm::vec3{c} / c.w};
        }

        inline Plane operator * (Matrix4 const& a, Plane const& b)
        {
            auto nd = Vector4{b.normal, b.d} * a.inverse();
            auto normal = Vector3{nd};
            auto dist = nd.w / normal.length();
            return Plane{normal.hat(), dist};
        }

        inline std::ostream & operator << (std::ostream & o, Matrix4 const& m)
        {
            return (o << "Matrix4{\n"
                << "    " << Vector4{m[0]} << ",\n"
                << "    " << Vector4{m[1]} << ",\n"
                << "    " << Vector4{m[2]} << ",\n"
                << "    " << Vector4{m[3]} << ",\n"
                << "}");
        }
    }
}

