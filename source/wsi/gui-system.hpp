#pragma once
#include "../lib/gl.hpp"
#include "../util/constraint.hpp"

namespace ss
{
    inline namespace wsi
    {
        struct GUI_System final: util::Non_Transferable
        {
            GUI_System(GLFWwindow* window);
            ~GUI_System();

            void render();
            void new_frame();
        };
    }
}

