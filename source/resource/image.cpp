#include "../util/path.hpp"
#include "image.hpp"
#include <utility>      // for std::move
#include <stdexcept>
#include <cassert>

#define STB_IMAGE_IMPLEMENTATION
#define STBI_FAILURE_USERMSG
#include "../third-party/stb/stb_image.h"

#include "../util/unreachable.macro.hpp"

#include "../util/compat/os.macro.hpp"
#ifdef SS_UTIL_ANDROID_COMPATIBLE
    #include "../util/compat/os.undef.hpp"
    #include "../lib/native.hpp"

    namespace ss
    {
        namespace
        {
            namespace stb_android_io
            {
                auto read(void* user, char* buf, int size) -> int
                {
                    auto asset = static_cast<android::Asset*>(user);
                    return asset->read(buf, size);
                };

                void skip(void* user, int n)
                {
                    auto asset = static_cast<android::Asset*>(user);
                    return asset->seek(n);
                };

                auto eof(void* user) -> int
                {
                    auto asset = static_cast<android::Asset*>(user);
                    return asset->eof();
                };

                const stbi_io_callbacks callbacks{read, skip, eof};
            }
        }
    }

    #define STBI_LOAD_FROM_ANDROID(VARIANT, PATH, ...) ([&] { \
        ::ss::android::Asset asset{static_cast<char const*>(PATH)}; \
        auto user = static_cast<void*>(&asset); \
        return stbi_##VARIANT##_from_callbacks( \
            &::ss::stb_android_io::callbacks, user, __VA_ARGS__); \
    } ())

    #define stbi_load(...) STBI_LOAD_FROM_ANDROID(load, __VA_ARGS__)
    #define stbi_loadf(...) STBI_LOAD_FROM_ANDROID(loadf, __VA_ARGS__)
#else
    #include "../util/compat/os.undef.hpp"
#endif

namespace ss
{
    namespace resource
    {
        auto operator << (std::ostream& o, Image_Layout il) -> std::ostream&
        {
            switch (il) {
                case Image_Layout::u8x1: return (o << "u8x1");
                case Image_Layout::u8x2: return (o << "u8x2");
                case Image_Layout::u8x3: return (o << "u8x3");
                case Image_Layout::u8x4: return (o << "u8x4");

                case Image_Layout::f32x1: return (o << "f32x1");
                case Image_Layout::f32x2: return (o << "f32x2");
                case Image_Layout::f32x3: return (o << "f32x3");
                case Image_Layout::f32x4: return (o << "f32x4");

                default: SS_UTIL_UNREACHABLE();
            }
        }

        auto component_count(Image_Layout il) -> int
        {
            switch (il) {
                case Image_Layout:: u8x1: // [[fallthrough]];
                case Image_Layout::f32x1: return 1;

                case Image_Layout:: u8x2: // [[fallthrough]];
                case Image_Layout::f32x2: return 2;

                case Image_Layout:: u8x3: // [[fallthrough]];
                case Image_Layout::f32x3: return 3;

                case Image_Layout:: u8x4: // [[fallthrough]];
                case Image_Layout::f32x4: return 4;

                default: SS_UTIL_UNREACHABLE();
            }
        }

        auto is_f32(Image_Layout il) -> bool
        {
            switch (il) {
                case Image_Layout::f32x1: // [[fallthrough]];
                case Image_Layout::f32x2: // [[fallthrough]];
                case Image_Layout::f32x3: // [[fallthrough]];
                case Image_Layout::f32x4: return true;

                case Image_Layout::u8x1: // [[fallthrough]];
                case Image_Layout::u8x2: // [[fallthrough]];
                case Image_Layout::u8x3: // [[fallthrough]];
                case Image_Layout::u8x4: return false;

                default: SS_UTIL_UNREACHABLE();
            }
        }

        auto bytes_per_component(Image_Layout il) -> int
        {
            return (is_f32(il) ? 4 : 1);
        }

        auto operator << (std::ostream& o, Image_Vertical_Flip vf) -> std::ostream&
        {
            switch (vf) {
                case Image_Vertical_Flip::no: return (o << "no-vertical-flip");
                case Image_Vertical_Flip::yes: return (o << "vertical-flip");
                default: SS_UTIL_UNREACHABLE();
            }
        }

        auto should_flip_vertically(Image_Vertical_Flip vf) -> bool
        {
            switch (vf) {
                case Image_Vertical_Flip::no: return false;
                case Image_Vertical_Flip::yes: return true;
                default: SS_UTIL_UNREACHABLE();
            }
        }

        Image::Image(int w, int h, buffer_type buf, Image_Layout il, std::string source)
            : w{w}
            , h{h}
            , buf{std::move(buf)}
            , il{il}
            , src{std::move(source)}
        {}

        auto Image::load(util::As_CStr cpath, Image_Layout il, Image_Vertical_Flip vf) -> Image*
        {
            auto path = util::Sanitized_Runtime_Path{cpath}.unsafe_take();
            if (auto img = cache().find(path, il, vf))
                return img;

            // FIXME: Either stb-image results in upside-down or FreeImage results in upside-down.
            // The engine has used FreeImage, then switched to stb-image.
            // One-more-flip works around the bug, but may have performance impact.
            // Should (maybe) fix the engine?
            stbi_set_flip_vertically_on_load(!should_flip_vertically(vf));

            auto comps = component_count(il);
            int w;
            int h;
            auto raw_buffer = (
                is_f32(il)
                ? (unsigned char*)stbi_loadf(path.data(), &w, &h, nullptr, comps)
                : stbi_load(path.data(), &w, &h, nullptr, comps)
            );

            if (raw_buffer == nullptr) {
                throw std::runtime_error{
                    "Failed to load image " + path + ": " + stbi_failure_reason()
                };
            }

            return cache().emplace(
                // cache keys
                std::move(path), il, vf,
                // image construction arguments
                w, h,
                buffer_type{raw_buffer, &stbi_image_free},
                il,
                cpath
            );
        }
    }
}

