#pragma once
// Android polyfill:
// - Emulate GLFW
// - Provide file IO for assets in APK
#include "../util/compat/os.macro.hpp"
#ifdef SS_UTIL_ANDROID_COMPATIBLE
#include "../util/compat/os.undef.hpp"

#include "../util/as-cstr.hpp"
#include <memory>

namespace ss
{
    inline namespace polyfill
    {
        namespace android
        {
            struct Asset_Internal_State;

            // Assets are read-only binary files.
            struct Asset
            {
                static auto slurp(util::As_CStr path) -> std::string;
                static auto readable(util::As_CStr path) -> bool;

                Asset(util::As_CStr path);

                // read `size` bytes into `buf`.
                // returns number of bytes actually read.
                // returns 0 when eof.
                auto read(void* buf, int size) -> int;

                auto size() -> int;
                auto tell() -> int;

                void seek(int offset);          // seek at current location
                void seek_front();              // seek to the beginning of the file
                void seek_back();               // seek to the end of file

                // are we at the end of file?
                auto eof() const -> bool;
                operator bool () const { return eof(); }

                ~Asset();

            private:
                std::unique_ptr<Asset_Internal_State> state;
            };
        }
    }
}

#else
    #include "../util/compat/os.undef.hpp"
#endif

