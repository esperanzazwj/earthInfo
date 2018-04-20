#pragma once
#include "app.hpp"
#include <string>
#include <memory>
#include <utility>      // for std::move
#include <functional>

namespace ss
{
    namespace app
    {
        struct Metadata final
        {
            using maker_fn_type = std::unique_ptr<App> ();
            using maker_type = std::function<maker_fn_type>;
            using name_type = std::string;

            maker_type maker;
            name_type name;
            name_type title;
            int width;
            int height;

            Metadata(maker_type maker, name_type name, name_type title, int width, int height)
                : maker{std::move(maker)}
                , name{std::move(name)}
                , title{std::move(title)}
                , width{width}
                , height{height}
            {}

            auto make() const -> std::unique_ptr<App> { return maker(); }
        };
    }
}

