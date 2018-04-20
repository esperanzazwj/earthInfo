#pragma once
#include "../util/tuple.hpp"
#include "../util/named.hpp"
#include <unordered_map>
#include <string>
#include <tuple>
#include <utility>      // for std::move and std::forward

namespace ss
{
    namespace resource
    {
        template <class Name_Trait, class Value, class ...Keys>
        struct Cache: util::Named<Name_Trait>
        {
            using value_type = Value;
            using keys_type = std::tuple<Keys...>;
            using container_type = std::unordered_map<keys_type, value_type, util::tuple_hash>;

            auto iter_of(Keys const&... keys)
            {
                return container.find(std::tuple<Keys const&...>{keys...});
            }

            auto find(Keys const&... keys) -> value_type*
            {
                auto it = iter_of(keys...);
                return (it == end(container) ? nullptr : &it->second);
            }

            auto contains(Keys const&... keys) -> bool
            {
                return (find(keys...) != nullptr);
            }

            // Construct a new cache entry in place.
            // Returns a pointer to the newly constructed cache value.
            template <class ...Args>
            auto emplace(Keys... keys, Args... args) -> value_type*
            {
                if (contains(keys...))
                    throw std::logic_error{"Duplicated " + this->name() + " cache key: " + util::tuple_to_string(std::move(keys)...)};

                auto result = container.emplace(
                    keys_type{std::move(keys)...},
                    value_type{std::forward<Args>(args)...}
                );

                return &result.first->second;
            }

            // Move in a value to cache under the keys.
            // Return a pointer to the newly moved cache value.
            auto add(Keys... keys, value_type x) -> value_type*
            {
                return emplace(std::move(keys)..., std::move(x));
            }

            // Move the matching value out of the cache.
            // Effectively delete it.
            auto take(Keys const&... keys) -> value_type
            {
                auto it = iter_of(keys...);
                if (it == end(container))
                    throw std::logic_error{"Cannot take unknown key in " + this->name() + " cache: " + util::tuple_to_string(keys...)};

                auto result = std::move(it->second);
                container.erase(it);
                return result;
            }

            // Delete a matching cache entry.
            // This may be more efficient than take(...) if you do not need the value.
            void remove(Keys const&... keys)
            {
                auto it = iter_of(keys...);
                if (it == end(container))
                    throw std::logic_error{"Cannot remove unknown key in " + this->name() + " cache: " + util::tuple_to_string(keys...)};

                container.erase(it);
            }

            void reset() { container.clear(); }

        private:
            container_type container;
        };
    }
}

