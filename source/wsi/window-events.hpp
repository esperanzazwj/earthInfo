#pragma once
#include "../lib/gl.hpp"
#include "event-source.hpp"

namespace ss
{
    inline namespace wsi
    {
        namespace window_events
        {
            using frame = Static_Event_Source<struct frame_tag>;
            using key = Static_Event_Source<struct key_tag, GLFWwindow*, int, int, int, int>;
            using mouse_button = Static_Event_Source<struct mouse_button_tag, GLFWwindow*, int, int, int>;
            using mouse_move = Static_Event_Source<struct mouse_move_tag, GLFWwindow*, double, double>;
            using scroll = Static_Event_Source<struct scroll_tag, GLFWwindow*, double, double>;
            using char_input = Static_Event_Source<struct char_input_tag, GLFWwindow*, unsigned int>;
            using focus = Static_Event_Source<struct focus_tag, GLFWwindow*, int>;
            using iconify = Static_Event_Source<struct iconify_tag, GLFWwindow*, int>;
        }
    }
}

