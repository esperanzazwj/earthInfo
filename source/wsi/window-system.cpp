#include "../lib/gl.hpp"
#include "../lib/native.hpp"
#include "window-system.hpp"
#include "window-events.hpp"
#include <stdexcept>

#include "../util/compat/os.macro.hpp"

namespace ss
{
    inline namespace wsi
    {
        namespace
        {
            #ifdef SS_UTIL_WEB_COMPATIBLE
                void static_on_frame(void* raw_ws)
                {
                    auto ws = static_cast<Window_System*>(raw_ws);
                    ws->on_frame();
                }
            #endif
        }

        Window_System::Window_System(int w, int h, char const* title)
            : ctx{w, h, title}
            , gui{ctx}
        {
            glfwGetFramebufferSize(ctx, &framebuffer_width, &framebuffer_height);

            glfwSetKeyCallback(ctx, window_events::key::emit);
            glfwSetMouseButtonCallback(ctx, window_events::mouse_button::emit);
            glfwSetCursorPosCallback(ctx, window_events::mouse_move::emit);
            glfwSetScrollCallback(ctx, window_events::scroll::emit);
            glfwSetWindowFocusCallback(ctx, window_events::focus::emit);
            glfwSetWindowIconifyCallback(ctx, window_events::iconify::emit);
            glfwSetCharCallback(ctx, window_events::char_input::emit);

            #ifndef SS_UTIL_WEB_COMPATIBLE
                glfwSwapInterval(0);
            #endif
        }

        Window_System::~Window_System()
        {
            #ifdef SS_UTIL_WEB_COMPATIBLE
                if (mainloop_has_run)
                    emscripten_cancel_main_loop();
            #endif
        }

        void Window_System::mainloop(app::App* app)
        {
            if (mainloop_has_run)
                throw std::logic_error{"ws.mainloop() can only be ran once."};
            mainloop_has_run = true;

            this->app = app;

            #ifdef SS_UTIL_WEB_COMPATIBLE
                emscripten_set_main_loop_arg(
                    static_on_frame,
                    static_cast<void*>(this),
                    -1,         // fps: depends on requestAnimationFrame (usually depends on vsync)
                    true        // simulate_infinite_loop: mimic the C++ behaviors
                );
            #else
                while (!glfwWindowShouldClose(ctx))
                    on_frame();
            #endif
        }

        void Window_System::on_frame()
        {
            gui.new_frame();
            window_events::frame::emit();
            app->render();
            gui.render();

            glfwSwapBuffers(ctx);
            glfwPollEvents();
        }

        double Window_System::seconds() const
        {
            return glfwGetTime();
        }
    }
}

