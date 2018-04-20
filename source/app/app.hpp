#pragma once
#include "../util/base-class.hpp"

namespace ss
{
    namespace app
    {
        struct App: util::Base_Class
        {
            virtual void render() = 0;
        };
    }
}

