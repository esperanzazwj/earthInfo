#include "show-error.hpp"
#include "compat/os.macro.hpp"
#include <iostream>
#include <typeinfo>

#if defined(SS_UTIL_WINDOWS_COMPATIBLE)
    #include "../lib/native.hpp"
    namespace
    {
        void show_error_message_box(std::string const& message)
        {
            MessageBox(nullptr, message.c_str(), "Error", MB_OK | MB_ICONERROR);
        }
    }
#elif defined(SS_UTIL_WEB_COMPATIBLE)
    #include "../lib/native.hpp"
    namespace
    {
        void show_error_message_box(std::string const& message)
        {
            EM_ASM_({
                var error = UTF8ToString($0);
                alert(error);
                throw error;
            }, message.data());
        }
    }
#else
    namespace
    {
        void show_error_message_box(std::string const& /*message*/)
        {
            // do nothing intensionally
        }
    }
#endif

#ifdef SS_UTIL_LINUX_COMPATIBLE
    namespace
    {
        void print_error_message(std::string const& message)
        {
            std::cerr << "\e[1;31mError: \e[0;31m" << message << "\e[0m\n";
        }
    }
#else
    namespace
    {
        void print_error_message(std::string const& message)
        {
            std::cerr << "Error: " << message << "\n";
        }
    }
#endif

namespace ss
{
    namespace util
    {
        void show_error(std::string const& message)
        {
            print_error_message(message);
            show_error_message_box(message);
        }

        void show_error(std::exception const& e)
        {
            std::string type = typeid(e).name();
            show_error(type + "\n\n" + e.what());
        }
    }
}

