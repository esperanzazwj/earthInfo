#include "../../lib/gl.hpp"
#include "../../wsi/window-system.hpp"
#include "../../third-party/imgui/imgui-all.hpp"
#include "dummy.hpp"

namespace ss
{
    namespace app
    {
        namespace apps
        {
            void Dummy::render()
            {
                auto& ws = wsi::Window_System::current();
                auto now = ws.seconds();
                auto nowi = int(now * 1000.0);
                auto a = float(nowi % 1000) / 1000.0f;
                glClearColor(0.0f, a, 0.0f, 1.0f);
                glClear(GL_COLOR_BUFFER_BIT);

                static auto show_text = 0.0;

                if (ImGui::Button("Hello"))
                    show_text = now;

                constexpr auto duration = 1.5;
                if (now - show_text < duration) {
                    ImGui::SameLine();
                    ImGui::Text("%s - %f", "World!", 1.0 - (now - show_text) / duration);
                }
            }
        }
    }
}

