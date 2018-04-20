#include "string.hpp"
#include <algorithm>

namespace ss
{
    namespace util
    {
        inline namespace string
        {
            bool starts_with(std::string const& haystack, std::string const& needle)
            {
                if (needle.size() > haystack.size()) return false;
                return std::equal(begin(needle), end(needle), begin(haystack));
            }
        }
    }
}

