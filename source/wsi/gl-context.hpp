#pragma once
#include "../lib/gl.hpp"
#include "../util/constraint.hpp"

namespace ss
{
    inline namespace wsi
    {
        struct GL_Context final: util::Non_Transferable
        {
            GL_Context(int w, int h, char const* title);
            ~GL_Context();

            GLFWwindow* handle() const { return window; }
            operator GLFWwindow* () const { return handle(); }

            // You can only assume OpenGL ES 3.0, GLSL 3.00 ES
            bool limited() const { return limited_; }

        private:
            GLFWwindow* window{};
            bool limited_{};
        };
    }
}

