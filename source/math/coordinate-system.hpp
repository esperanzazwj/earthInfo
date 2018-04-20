#pragma once
#include "vector3.hpp"
#include <cmath>

namespace ss
{
    namespace math
    {
        inline namespace coordinate_system
        {
            inline void CoordinateSystem0(Vector3 const& v1, Vector3 & v2, Vector3 & v3)
            {
                if (v1.y != 1 && v1.y != -1) {
                    v2 = Vector3{v1.z, 0, -v1.x}.hat();
                    v3 = v1.crossProduct(v2);
                } else {
                    v2 = Vector3{1, 0, 0};
                    v3 = Vector3{0, 0, 1};
                }
            }

            inline void CoordinateSystem1(Vector3 const& v1, Vector3 & v2, Vector3 & v3)
            {
                if (std::abs(v1.x) > std::abs(v1.y)) {
                    float invLen = 1.0f / std::sqrt(v1.x*v1.x + v1.z*v1.z);
                    v2 = Vector3{-v1.z * invLen, 0.0f, v1.x * invLen};
                }
                else {
                    float invLen = 1.0f / std::sqrt(v1.y*v1.y + v1.z*v1.z);
                    v2 = Vector3{0.0f, v1.z * invLen, -v1.y * invLen};
                }
                v3 = v1.crossProduct(v2);
            }

            inline void CoordinateSystem(Vector3 const& v1, Vector3 & v2, Vector3 & v3)
            {
                // using stereographi projection parameterization
                // notice it could be the singular point (0, -1, 0)

                if ((v1.y + 1.0f) < 1.0e-8f) {
                    v2 = Vector3{0, 0, 0};
                } else {
                    v2 = Vector3{
                        (v1.y + 1.0f) * (v1.y + 1.0f) + v1.z * v1.z - v1.x * v1.x,
                        -2.0f * v1.x * (v1.y + 1.0f),
                        -2.0f * v1.x * v1.z,
                    }.hat();
                }
                v3 = v1.crossProduct(v2);
            }
        }
    }
}

