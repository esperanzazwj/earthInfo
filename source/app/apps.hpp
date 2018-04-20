#pragma once
#include "registry.hpp"

namespace ss
{
    namespace app
    {
        void register_all_apps(Registry& reg);
        auto register_all_apps() -> Registry;
    }
}

