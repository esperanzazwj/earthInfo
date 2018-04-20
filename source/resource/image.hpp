#pragma once
#include "../util/singleton.hpp"
#include "../util/as-cstr.hpp"
#include "cache.hpp"
#include <string>
#include <memory>
#include <iostream>

namespace ss
{
    namespace resource
    {
        enum struct Image_Layout
        {
            u8x1,
            u8x2,
            u8x3,
            u8x4,

            f32x1,
            f32x2,
            f32x3,
            f32x4,
        };

        auto operator << (std::ostream& o, Image_Layout il) -> std::ostream&;
        auto component_count(Image_Layout il) -> int;
        auto is_f32(Image_Layout il) -> bool;
        auto bytes_per_component(Image_Layout il) -> int;

        enum struct Image_Vertical_Flip
        {
            no,
            yes,
        };

        auto operator << (std::ostream& o, Image_Vertical_Flip vf) -> std::ostream&;
        auto should_flip_vertically(Image_Vertical_Flip vf) -> bool;

        struct Image
        {
            struct Name
            {
                static constexpr auto name() { return "image"; }
            };

            using cache_type = Cache<Name, Image, std::string, Image_Layout, Image_Vertical_Flip>;
            using singleton_cache_type = util::Singleton<cache_type>;
            using buffer_type = std::unique_ptr<unsigned char[], void (*)(void*)>;

            Image(int w, int h, buffer_type buf, Image_Layout il, std::string source);

            auto image_layout() const -> Image_Layout { return il; }
            auto width() const -> int { return w; }
            auto height() const -> int { return h; }

            auto& source() const { return src; }
            auto data() const { return buf.get(); }
            auto data()       { return buf.get(); }

            static auto cache() -> cache_type& { return singleton_cache_type::instance(); }

            static auto load(util::As_CStr path, Image_Layout il=Image_Layout::u8x3, Image_Vertical_Flip vf=Image_Vertical_Flip::no) -> Image*;

        private:
            int w;
            int h;
            buffer_type buf;
            Image_Layout il;
            std::string src;
        };
    }
}

