#pragma once
// Used in function parameter to receive either a C-style string or C++ string efficiently.
#include <string>

namespace ss
{
    namespace util
    {
        struct As_CStr
        {
            As_CStr(char const* s): cstr{s} {}
            As_CStr(std::string const& s): cstr{s.data()} {}

            auto as_cstr() const -> char const* { return cstr; }
            auto clone_as_str() const -> std::string { return cstr; }
            operator char const* () const { return as_cstr(); }
            operator std::string () const { return clone_as_str(); }

        private:
            char const* cstr;
        };
    }
}

