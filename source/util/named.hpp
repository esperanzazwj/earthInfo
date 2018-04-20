#pragma once
#include <string>

namespace ss
{
    namespace util
    {
        template <class Name_Trait>
        struct Named
        {
            using name_trait = Name_Trait;

            static constexpr auto raw_name() noexcept -> char const* { return name_trait::name(); }
            static auto name() -> std::string { return raw_name(); }
        };
    }
}

