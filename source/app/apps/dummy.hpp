#pragma once
#include "../app.hpp"

namespace ss
{
    namespace app
    {
        namespace apps
        {
            struct Dummy: App
            {
                void render() override;
            };
        }
    }
}

