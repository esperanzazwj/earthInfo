#pragma once
#include "../app.hpp"
#include <memory>

namespace ss
{
    namespace app
    {
        namespace apps
        {
            struct Globe_Impl;

            struct Globe: App
            {
                Globe();
                ~Globe() override;

                void render() override;

            private:
                std::unique_ptr<Globe_Impl> impl;
            };
        }
    }
}

