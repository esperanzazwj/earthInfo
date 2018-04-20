#include "file.hpp"
#include <fstream>
#include <sstream>

#include "compat/os.macro.hpp"
#ifdef SS_UTIL_ANDROID_COMPATIBLE
    #include "compat/os.undef.hpp"
    #include "../lib/native.hpp"
    #include "compat/os.macro.hpp"
#endif

namespace ss
{
    namespace util
    {
        inline namespace file
        {
            std::string slurp(Sanitized_Runtime_Path const& path)
            {
                #ifdef SS_UTIL_ANDROID_COMPATIBLE
                    return android::Asset::slurp(path.unsafe_get());
                #else
                    if (std::ifstream ifs{path.unsafe_get(), std::ios::binary}) {
                        std::stringstream ss;
                        ss << ifs.rdbuf();
                        return ss.str();
                    }
                    else {
                        throw std::runtime_error{"Cannot load file: " + path.unsafe_get_as_str()};
                    }
                #endif
            }
        }
    }
}

