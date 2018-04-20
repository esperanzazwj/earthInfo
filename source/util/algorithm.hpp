#pragma once
// Extending the standard <algorithm>
#include <algorithm>
#include <type_traits>

namespace ss
{
    namespace util
    {
        inline namespace algorithm
        {
            // Bitwise-copy the first few bytes to an array of something.
            // Source must be trivially copyable.
            // Only the min(sizeof(src), sizeof(dst)) is copied.
            template <class Source, class Destination_Element, int N>
            void copy_bits(Source const& src, Destination_Element (&dst)[N])
            {
                static_assert(
                    std::is_trivially_copyable<Source>::value,
                    "Source type must be trivially copyable, "
                    "that is, can be safely memmove'ed.");
                static_assert(
                    std::is_trivially_copyable<Destination_Element>::value,
                    "Destination type must be an array of trivially copyable values, "
                    "that is, can be safely memmove'ed.");

                constexpr auto src_size = sizeof(src);
                constexpr auto dst_size = sizeof(dst);
                constexpr auto min_size = src_size < dst_size ? src_size : dst_size;

                auto s = reinterpret_cast<char const*>(&src);
                auto d = reinterpret_cast<char*>(&dst);
                std::copy_n(s, min_size, d);
            }
        }
    }
}

