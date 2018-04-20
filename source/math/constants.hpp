#pragma once
#include <limits>

namespace ss
{
    namespace math
    {
        inline namespace constants
        {
            static constexpr float inf = std::numeric_limits<float>::infinity();
            static constexpr float eps = 1e-4f;

            static constexpr float e             = 2.7182818284590452354f;       // base of natural logarithm
            static constexpr float ln2           = 0.69314718055994530942f;      // log(2) / log(e)
            static constexpr float ln10          = 2.30258509299404568402f;      // log(10) / log(e)
            static constexpr float inv_ln2       = 1.4426950408889634074f;       // log(e) / log(2)
            static constexpr float inv_ln10      = 0.43429448190325182765f;      // log(e) / log(10)
            static constexpr float pi            = 3.14159265358979323846f;      // π
            static constexpr float pi_2          = 1.57079632679489661923f;      // π / 2
            static constexpr float pi_4          = 0.78539816339744830962f;      // π / 4
            static constexpr float inv_pi        = 0.31830988618379067154f;      // 1 / π
            static constexpr float _2inv_pi      = 0.63661977236758134308f;      // 2 / π
            static constexpr float _2inv_sqrt_pi = 1.12837916709551257390f;      // 2 / sqrt(π)
            static constexpr float sqrt_2        = 1.41421356237309504880f;      // sqrt(2)
            static constexpr float inv_sqrt_2    = 0.70710678118654752440f;      // 1 / sqrt(2)
        }
    }
}

