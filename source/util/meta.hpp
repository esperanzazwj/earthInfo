#pragma once
#include <tuple>

namespace ss
{
    namespace util
    {
        namespace meta
        {
            namespace discard_values_detail
            {
                inline constexpr void impl() {}

                template <class T, class ...Ts>
                constexpr void impl(T x, Ts ...xs)
                {
                    (void)x;
                    impl(xs...);
                }

                // discard_values(xs...) = void
                //
                // Pretend every argument does not exists.
                // This also silent the warning of unused variables.
                template <class ...Ts>
                constexpr void discard_values(Ts ...xs)
                {
                    impl(xs...);
                }
            }
            using discard_values_detail::discard_values;

            // nth_value<0>(x, xs...) = x
            // nth_value<N>(x, xs...) = nth_value<N-1>(xs...)
            // N is size_t
            template <size_t N, class T, class ...Ts>
            constexpr auto nth_value(T x, Ts ...xs)
            {
                using std::get;
                return get<N>(std::tuple<T, Ts...>(x, xs...));
            }

            // front_value(x, xs...) = x
            template <class T, class ...Ts>
            constexpr auto front_value(T x, Ts ...xs)
            {
                return nth_value<0>(x, xs...);
            }

            // back_value(xs..., x) = x
            template <class T, class ...Ts>
            constexpr auto back_value(T x, Ts ...xs)
            {
                return nth_value<sizeof...(Ts)>(x, xs...);
            }
        }
    }
}

