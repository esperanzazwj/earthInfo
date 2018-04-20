#include "util/path.hpp"
#include "wsi/window-system.hpp"
#include "app/apps.hpp"
#include <stdexcept>
#include <iostream>

#include "util/compat/os.macro.hpp"

#if defined(SS_UTIL_LINUX_COMPATIBLE)
    // Linux has nice error reporting for uncaught exceptions,
    // so just let exceptions slip through.
    #define TRY
    #define CATCH_AND_REPORT
#else
    #include "util/show-error.hpp"

    #if defined(SS_UTIL_WEB_COMPATIBLE)
        #define CATCH_EXCEPTION_TYPE std::exception
    #else
        #define CATCH_EXCEPTION_TYPE std::runtime_error
    #endif

    #define TRY try
    #define CATCH_AND_REPORT \
        catch (CATCH_EXCEPTION_TYPE const& e) { \
            ss::util::show_error(e); \
        }
#endif

int main() TRY
{
    ss::util::init_path();

    auto apps = ss::app::register_all_apps();
    auto names = apps.collect_names();

    std::cout << "Available apps:\n";
    for (auto& name: names)
        std::cout << "- " << name << "\n";
    if (names.empty()) {
        std::cout << "  none\n";
        return 0;
    }

    auto md = apps.metadata_of("globe");
    std::cout << "Running " << md->name << "\n";

    ss::wsi::Window_System ws{md->width, md->height, md->title.data()};
    auto app = md->make();
    ws.mainloop(app.get());
}
CATCH_AND_REPORT

