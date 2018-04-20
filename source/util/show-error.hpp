#pragma once
#include <string>
#include <stdexcept>

namespace ss
{
    namespace util
    {
        // Popup a message box to show error messages
        void show_error(std::string const& message);
        void show_error(std::exception const& e);
    }
}

