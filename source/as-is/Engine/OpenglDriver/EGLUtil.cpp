#include "EGLUtil.h"
#include <unordered_map>
#include <string>

namespace ss
{
    namespace egl
    {
        namespace
        {
            std::unordered_map<GLenum, std::string> error_strings{
                #define ENTRY(NAME) { GL_ ## NAME, #NAME }
                ENTRY(INVALID_ENUM),
                ENTRY(INVALID_VALUE),
                ENTRY(INVALID_OPERATION),
                ENTRY(INVALID_FRAMEBUFFER_OPERATION),
                ENTRY(OUT_OF_MEMORY),
                #undef ENTRY
            };

            std::string unknown_error{"Unknown Error"};
        }

        std::string const& error_to_string(GLenum error)
        {
            auto entry_it = error_strings.find(error);
            if (entry_it == error_strings.end()) return unknown_error;
            return entry_it->second;
        }
    }
}

