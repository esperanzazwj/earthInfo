#include "apps.hpp"
#include "apps/dummy.hpp"
#include "apps/globe.hpp"

namespace ss
{
    namespace app
    {
        void register_all_apps(Registry& reg)
        {
            reg.add<apps::Dummy>("dummy");
            reg.add<apps::Globe>("globe");
        }

        auto register_all_apps() -> Registry
        {
            Registry reg;
            register_all_apps(reg);
            return reg;
        }
    }
}

