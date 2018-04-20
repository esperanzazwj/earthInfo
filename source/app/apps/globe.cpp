#include "../../util/constraint.hpp"
#include "../../as-is/app/Globe.h"
#include "globe.hpp"

namespace ss
{
    namespace app
    {
        namespace apps
        {
            struct Globe_Impl: util::Non_Transferable
            {
                Globe_Impl()
                    : inner{std::make_unique<::app::Globe>()}
                {
                    inner->Init();
                }

                ~Globe_Impl()
                {
                    // FIXME: AppFrameWork2 does not release resources.
                }

                void render()
                {
                    inner->Render();
                }

            private:
                std::unique_ptr<::App> inner;
            };

            Globe::Globe()
                : impl{std::make_unique<Globe_Impl>()}
            {}

            Globe::~Globe() = default;

            void Globe::render()
            {
                impl->render();
            }
        }
    }
}

