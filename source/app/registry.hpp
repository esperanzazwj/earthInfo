#pragma once
#include "app.hpp"
#include "metadata.hpp"
#include <map>
#include <vector>
#include <string>
#include <memory>
#include <type_traits>

namespace ss
{
    namespace app
    {
        struct Registry final
        {
            using metadata_type = Metadata;
            using name_type = metadata_type::name_type;
            using metadata_list_type = std::map<name_type, metadata_type>;

            auto metadata_of(std::string const& name) const -> metadata_type const*;
            auto collect_names() const -> std::vector<std::string>;
            void add(metadata_type md);

            template <class Concrete_App>
            void add(name_type name, int width, int height, name_type title)
            {
                static_assert(
                    std::is_base_of<App, Concrete_App>::value,
                    "You can only register apps that derive from App."
                );

                add(metadata_type{
                    [] () -> std::unique_ptr<App> {
                        return std::make_unique<Concrete_App>();
                    },
                    std::move(name),
                    std::move(title),
                    width,
                    height,
                });
            }

            template <class Concrete_App>
            void add(name_type name, int width, int height)
            {
                auto title = name + " - ss Engine";
                add<Concrete_App>(std::move(name), width, height, std::move(title));
            }

            template <class Concrete_App>
            void add(name_type name, int resolution)
            {
                add<Concrete_App>(std::move(name), 16 * resolution, 9 * resolution);
            }

            template <class Concrete_App>
            void add(name_type name)
            {
                add<Concrete_App>(std::move(name), 64);
            }

        private:
            metadata_list_type metadatas;
        };
    }
}

