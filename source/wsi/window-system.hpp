#pragma once
#include "../app/app.hpp"
#include "../util/exclusive.hpp"
#include "gl-context.hpp"
#include "gui-system.hpp"

namespace ss
{
    inline namespace wsi
    {
        struct Window_System final: util::Exclusive<Window_System>
        {
            Window_System(int w, int h, char const* title);
            ~Window_System();

            void mainloop(app::App* app);
            void on_frame();
            double seconds() const;     // returns the current time in seconds. Uses highest-precision monotonic timer.
            auto& context() const { return ctx; }

            // Access framebuffer size. These may be different from the values passed into the constructor.
            auto width() const { return framebuffer_width; }
            auto height() const { return framebuffer_height; }

        private:
            GL_Context ctx;
            GUI_System gui;
            app::App* app{};
            bool mainloop_has_run{};
            int framebuffer_width;
            int framebuffer_height;
        };
    }
}

