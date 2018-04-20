#pragma once
#include "vector3.hpp"
#include "vector4.hpp"
#include <cereal/archives/json.hpp>
#include <cereal/cereal.hpp>

namespace ss
{
    namespace math
    {
        #define N(PREFIX, NAME) ::cereal::make_nvp(#NAME, (PREFIX).NAME)

        template <class Archive>
        void serialize(Archive & archive, Vector3 & m)
        {
            archive(N(m, x), N(m, y), N(m, z));
        }

        template <class Archive>
        void serialize(Archive & archive, Vector4 & m)
        {
            archive(N(m, x), N(m, y), N(m, z), N(m, w));
        }

        #undef N
    }
}

