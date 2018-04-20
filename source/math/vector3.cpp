#include "vector3.hpp"
#include <regex>
#include <stdexcept>

namespace ss
{
    namespace math
    {
        const Vector3 Vector3::ZERO{0.0f, 0.0f, 0.0f};
        const Vector3 Vector3::UNIT_SCALE{1.0f, 1.0f, 1.0f};
        const Vector3 Vector3::UNIT_X{1.0f, 0.0f, 0.0f};
        const Vector3 Vector3::UNIT_Y{0.0f, 1.0f, 0.0f};
        const Vector3 Vector3::UNIT_Z{0.0f, 0.0f, 1.0f};
        const Vector3 Vector3::NEGATIVE_UNIT_X{-1.0f, 0.0f, 0.0f};
        const Vector3 Vector3::NEGATIVE_UNIT_Y{0.0f, -1.0f, 0.0f};
        const Vector3 Vector3::NEGATIVE_UNIT_Z{0.0f, 0.0f, -1.0f};

        Vector3::Vector3(std::string const & xs)
        {
            static std::regex fmt{"([^\\s,]+)[\\s,]+([^\\s,]+)[\\s,]+([^\\s,]+)"};

            std::smatch components;
            if (!std::regex_search(xs, components, fmt))
                throw std::invalid_argument{"Cannot parse a Vector3 out of " + xs};

            v[0] = std::stof(components[1]);
            v[1] = std::stof(components[2]);
            v[2] = std::stof(components[3]);
        }
    }
}

