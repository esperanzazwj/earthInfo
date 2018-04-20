#pragma once
#include <iomanip>
#include <sstream>
#include <tuple>
#include <cstdint>

namespace ss
{
    namespace util
    {
        namespace tuple_hash_details
        {
            template <class T>
            inline auto hash_combine(std::size_t seed, T const& x) -> std::size_t
            {
                std::hash<T> hasher;
                return seed ^ (hasher(x) + 0x9e3779b9 + (seed << 6) + (seed >> 2));
            }

            template <class Tuple, int Index=int(std::tuple_size<Tuple>::value)-1>
            struct Impl
            {
                static auto apply(Tuple const& x) -> std::size_t
                {
                    auto seed = Impl<Tuple, Index-1>::apply(x);
                    return hash_combine(seed, std::get<Index>(x));
                }
            };

            template <class Tuple>
            struct Impl<Tuple, -1>
            {
                static auto apply(Tuple const&) -> std::size_t { return 0; }
            };

            template <class Tuple>
            auto hash(Tuple const& x) -> std::size_t
            {
                return Impl<Tuple>::apply(x);
            }
        }

        struct tuple_hash
        {
            template <class Tuple>
            auto operator () (Tuple const& x) const -> std::size_t
            {
                return tuple_hash_details::hash(x);
            }
        };

        namespace tuple_to_string_details
        {
            template <class T>
            inline auto to_string(T const& x) -> std::string
            {
                std::ostringstream oss;
                oss << x;
                return oss.str();
            }

            inline auto to_string(char x) -> std::string
            {
                return to_string(int(x));
            }

            inline auto to_string(unsigned char x) -> std::string
            {
                return to_string(int(x));
            }

            inline auto to_string(char const* x) -> std::string
            {
                std::ostringstream oss;
                oss << std::quoted(x);
                return oss.str();
            }

            inline auto to_string(std::string const& x) -> std::string
            {
                return to_string(x.data());
            }

            template <class Tuple, int Index=int(std::tuple_size<Tuple>::value)-1>
            struct Impl
            {
                static void append(std::string& s, Tuple const& x)
                {
                    Impl<Tuple, Index-1>::append(s, x);
                    s += ", ";
                    s += to_string(std::get<Index>(x));
                }
            };

            template <class Tuple>
            struct Impl<Tuple, 0>
            {
                static auto append(std::string& s, Tuple const& x)
                {
                    s += to_string(std::get<0>(x));
                }
            };

            template <class Tuple>
            struct Impl<Tuple, -1>
            {
                static auto append(std::string&, Tuple const&) {}
            };

            template <class ...Ts>
            auto tuple_to_string(std::tuple<Ts...> const& x) -> std::string
            {
                std::string result{"<"};
                Impl<std::tuple<Ts...>>::append(result, x);
                result += ">";
                return result;
            }

            template <class ...Ts>
            auto tuple_to_string(Ts const&... xs) -> std::string
            {
                return tuple_to_string(std::tuple<Ts const&...>{xs...});
            }
        }

        using tuple_to_string_details::tuple_to_string;
    }
}

