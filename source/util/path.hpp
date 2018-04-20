#pragma once
#include "../util/constraint.hpp"
#include "../util/as-cstr.hpp"
#include <string>
#include <vector>

namespace ss
{
    namespace util
    {
        inline namespace path
        {
            void init_path();

            bool is_absolute_path(util::As_CStr path);

            // basename("hello") -> "hello"
            // basename("hello/wo\\rld\\foo/bar") -> "bar"
            // basename("hello/wo\\rld\\foo.bar") -> "foo.bar"
            auto basename(util::As_CStr path) -> std::string;

            // Generate a list of filenames:
            // - prefix + "0" + suffix
            // - prefix + "1" + suffix
            // - prefix + "2" + suffix
            // - prefix + "3" + suffix
            // - prefix + "4" + suffix
            // - prefix + "5" + suffix
            std::vector<std::string> generate_cubemap_paths(std::string const& prefix, std::string const& suffix);

            struct Sanitized_Runtime_Path
            {
                // Pre-condition:
                // - path is a relative path
                // - path is not the result of Sanitized_Runtime_Path{some_other_path}.get*()
                //
                // Guarantees:
                // - pointer equality: this->unsafe_get_as_str().data() == this->unsafe_get()
                // - relative path: starts_with(this->unsafe_get(), "/") == false
                // - not directory: ends_with(this->unsafe_get(), "/") == false
                // - non-empty: this->unsafe_get_as_str().empty() == false
                Sanitized_Runtime_Path(util::As_CStr path);
                Sanitized_Runtime_Path(char const* path) : Sanitized_Runtime_Path{util::As_CStr{path}} {}
                Sanitized_Runtime_Path(std::string const& path) : Sanitized_Runtime_Path{util::As_CStr{path}} {}

                auto unsafe_get() const -> char const* { return sanitized.data(); }
                auto unsafe_get_as_str() const -> std::string const& { return sanitized; }
                auto unsafe_take() -> std::string { return std::move(sanitized); }

            private:
                std::string sanitized;
            };

            namespace internal
            {
                // Let `sanitize_runtime_path` look for files in the `dir` directory
                struct Working_Directory_Guard: util::Non_Transferable
                {
                    Working_Directory_Guard(std::string dir);
                    ~Working_Directory_Guard();
                };
            }

            // Let `sanitize_runtime_path` look for files in the same directory that `file` is in
            struct Working_Directory_File_Guard: util::Non_Transferable
            {
                Working_Directory_File_Guard(Sanitized_Runtime_Path file);

            private:
                internal::Working_Directory_Guard guard;
            };
        }
    }
}

