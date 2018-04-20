#include "../lib/gl.hpp"
#include "../third-party/imgui/imgui-all.hpp"
#include "../util/file.hpp"
#include "window-events.hpp"
#include "gui-system.hpp"

namespace ss
{
    inline namespace wsi
    {
        GUI_System::GUI_System(GLFWwindow* window)
        {
            ImGui_ImplGlfwGL3_Init(window, false);
            auto& io = ImGui::GetIO();

            // Disable file writing
            io.IniFilename = nullptr;
            io.LogFilename = nullptr;

            {
                auto font = util::slurp("assets/fonts/ui.ttf");
                ImFontConfig cfg;
                cfg.FontDataOwnedByAtlas = false;
                io.Fonts->AddFontFromMemoryTTF((void*)font.data(), int(font.size()), 15.0f, &cfg);
            }

            window_events::mouse_button::subscribe(ImGui_ImplGlfwGL3_MouseButtonCallback);
            window_events::scroll::subscribe(ImGui_ImplGlfwGL3_ScrollCallback);
            window_events::key::subscribe(ImGui_ImplGlfwGL3_KeyCallback);
            window_events::char_input::subscribe(ImGui_ImplGlfwGL3_CharCallback);
            window_events::focus::subscribe(ImGui_ImplGlfwGL3_FocusCallback);
        }

        GUI_System::~GUI_System()
        {
            ImGui_ImplGlfwGL3_Shutdown();
        }

        void GUI_System::render()
        {
            ImGui::Render();
        }

        void GUI_System::new_frame()
        {
            ImGui_ImplGlfwGL3_NewFrame();
        }
    }
}

