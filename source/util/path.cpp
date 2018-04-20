#include "../util/as-cstr.hpp"
#include "../lib/native.hpp"
#include "string.hpp"
#include "path.hpp"
#include <regex>
#include <stack>
#include <fstream>
#include <stdexcept>

#include "../util/assert.hpp"
#include "../util/compat/os.macro.hpp"

namespace ss
{
    namespace util
    {
        inline namespace path
        {
            namespace
            {
                bool initialized = false;
                std::stack<std::string> working_directories;

                #ifdef SS_UTIL_ANDROID_COMPATIBLE
                    const std::string runtime_prefix = "";
                #else
                    const std::string runtime_prefix = "runtime/";
                #endif

                std::string sanitize_runtime_path_unchecked(std::string path)
                {
                    // Strip runtime prefix if there is any
                    if (starts_with(path, runtime_prefix)) {
                        path = path.substr(runtime_prefix.size());
                    }

                    // Use forward slash: '\\' -> '/'
                    static std::regex backslash_re{"\\\\"};
                    path = regex_replace(path, backslash_re, "/");

                    // Guard two ends: "foo" -> "/foo/"
                    path = "/" + path + "/";

                    // Collapse consecutive slashes: "/foo///bar/" -> "/foo/bar/"
                    static std::regex consecutive_slashes_re{"/+"};
                    path = regex_replace(path, consecutive_slashes_re, "/");

                    // Simplify relative path: "/./hello/./../world/./" -> "/world/"
                    // The first slash may disappear: "/../" -> "/"
                    // - Remove "." section
                    static std::regex current_dir_re{"/\\.(?=/)"};
                    path = regex_replace(path, current_dir_re, "");
                    // - Remove ".." section
                    static std::regex parent_dir_re{"/[^/]+/\\.\\.(?=/)"};
                    for (std::string old_path; old_path != path;) {
                        old_path = std::move(path);
                        path = regex_replace(old_path, parent_dir_re, "", std::regex_constants::format_first_only);
                    }

                    // Remove last slash: "/world/" -> "/world", "/" -> ""
                    path.pop_back();

                    // Check whether path tries to go out of runtime path, or tries to get the runtime path
                    if (path.empty() || path == "/.." || starts_with(path, "/../")) {
                        throw std::runtime_error{
                            "You can only access files inside runtime path, "
                            "excluding the runtime path itself.\n"
                            "  Trying to access: " +
                            runtime_prefix + (path.size() == 0 ? "" : path.substr(1))
                        };
                    }

                    // Remove first slash: "/world" -> "world"
                    path = path.substr(1);

                    // Combine with runtime prefix: "world" -> "./runtime/world"
                    path = runtime_prefix + path;

                    return path;
                }

                namespace ensure_case_sensitive_path_details
                {
                    #if defined(SS_UTIL_WINDOWS_COMPATIBLE)
                    struct file_not_exists : std::runtime_error
                    {
                        using runtime_error::runtime_error;
                    };

                    std::string canonicalize_path(std::string const& path)
                    {
                        auto file = CreateFile(
                            path.data(),
                            GENERIC_READ,
                            FILE_SHARE_READ,
                            nullptr,
                            OPEN_EXISTING,
                            FILE_ATTRIBUTE_NORMAL,
                            nullptr);
                        if (file == INVALID_HANDLE_VALUE) throw file_not_exists{path};

                        auto size = GetFinalPathNameByHandle(file, nullptr, 0, FILE_NAME_NORMALIZED);
                        if (size == 0) {
                            CloseHandle(file);
                            throw std::logic_error{"WTF"};
                        }
                        std::string buffer(size + 1, 0);
                        GetFinalPathNameByHandle(file, &buffer[0], size + 1, FILE_NAME_NORMALIZED);
                        buffer.pop_back();  // remove '\0' at the end

                        CloseHandle(file);

                        return buffer.substr(4);    // strip \\?\ prefix
                    }

                    std::string get_full_path_name(std::string const& path)
                    {
                        auto size = GetFullPathName(path.c_str(), 0, nullptr, nullptr);
                        if (size == 0) throw file_not_exists{path};

                        std::string buffer(size, 0);
                        GetFullPathName(path.c_str(), size, &buffer[0], nullptr);
                        buffer.pop_back();  // remove '\0' at the end
                        return buffer;
                    }

                    std::string fix_path_casing(std::string const& path)
                    {
                        auto canonical_path = canonicalize_path(path);
                        auto full_runtime_prefix = get_full_path_name(runtime_prefix);
                        if (!starts_with(canonical_path, full_runtime_prefix)) {
                            throw std::runtime_error{
                                "Case-sensitive filenames are being ENFORCED.\n"
                                "\n"
                                "You are accessing file:\n"
                                "    " + canonical_path + "\n"
                                "which is not in the runtime directory:\n"
                                "    " + full_runtime_prefix + "\n"
                                "\n"
                                "Please fix your buggy code.\n"
                            };
                        }
                        auto path_body = canonical_path.substr(full_runtime_prefix.size());
                        return sanitize_runtime_path_unchecked(path_body);
                    }

                    void ensure_case_sensitive_path(std::string const& path)
                    {
                        auto fixed_path = fix_path_casing(path);
                        if (fixed_path != path) {
                            throw std::runtime_error{
                                "Case-sensitive filenames are being ENFORCED.\n"
                                "\n"
                                "You are accessing file:\n"
                                "    " + path + "\n"
                                "whose correct spelling is:\n"
                                "    " + fixed_path + "\n"
                                "\n"
                                "Please fix your buggy code.\n"
                            };
                        }
                    }
                    #elif defined(SS_UTIL_ANDROID_COMPATIBLE)
                    void ensure_case_sensitive_path(std::string const& path)
                    {
                        // Now that the Windows' version requires the file to exist,
                        // let's check the file readability in other systems at least.
                        if (!android::Asset::readable(path))
                            throw std::runtime_error{"File not exists or not readable: " + path};
                    }
                    #else
                    void ensure_case_sensitive_path(std::string const& path)
                    {
                        // Now that the Windows' version requires the file to exist,
                        // let's check the file readability in other systems at least.
                        std::ifstream ifs{path};
                        if (!ifs) throw std::runtime_error{"File not exists or not readable: " + path};
                    }
                    #endif
                }
                using ensure_case_sensitive_path_details::ensure_case_sensitive_path;

                std::string sanitize_runtime_path(std::string path)
                {
                    assert(initialized);

                    if (is_absolute_path(path)) {
                        throw std::runtime_error{"Only support relative path, but got an absolute path: " + path};
                    }

                    path = working_directories.top() + "/" + path;
                    path = sanitize_runtime_path_unchecked(path);
                    ensure_case_sensitive_path(path);
                    return path;
                }

                std::string directory_of_file(Sanitized_Runtime_Path file)
                {
                    auto path = file.unsafe_take().substr(runtime_prefix.size());

                    static std::regex basename_re{"(?:^|[/\\\\])[^/\\\\]+$"};
                    return regex_replace(path, basename_re, "");
                }
            }

            void init_path()
            {
                assert(runtime_prefix.empty() || runtime_prefix.back() == '/');

                initialized = true;
                static internal::Working_Directory_Guard root_directory_guard{""};
            }

            bool is_absolute_path(util::As_CStr path)
            {
                static std::regex abs_re{"^[a-zA-Z]:|^\\\\|^/"};
                return regex_search(path.as_cstr(), abs_re);
            }

            auto basename(util::As_CStr path) -> std::string
            {
                static std::regex stem_re{".*[/|\\\\]"};
                return regex_replace(path.as_cstr(), stem_re, "");
            }

            std::vector<std::string> generate_cubemap_paths(std::string const& prefix, std::string const& suffix)
            {
                assert(initialized);

                std::vector<std::string> paths(6, prefix + '0' + suffix);

                auto counter_at = prefix.length();
                auto counter = '0';
                for (auto& path: paths) path[counter_at] = counter++;

                return paths;
            }

            Sanitized_Runtime_Path::Sanitized_Runtime_Path(util::As_CStr path)
                : sanitized{sanitize_runtime_path(path)}
            {
            }

            namespace internal
            {
                Working_Directory_Guard::Working_Directory_Guard(std::string dir)
                {
                    assert(initialized);
                    working_directories.emplace(std::move(dir));
                }

                Working_Directory_Guard::~Working_Directory_Guard()
                {
                    working_directories.pop();
                }
            }

            Working_Directory_File_Guard::Working_Directory_File_Guard(Sanitized_Runtime_Path file)
                : guard{directory_of_file(std::move(file))}
            {}
        }
    }
}

