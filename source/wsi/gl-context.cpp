#include "../gl/init.hpp"
#include "gl-context.hpp"
#include <stdexcept>

#include "../util/compat/gl.macro.hpp"

namespace ss
{
    inline namespace wsi
    {
        GL_Context::GL_Context(int w, int h, char const* title)
        {
            if (!glfwInit()) throw std::runtime_error{"failed to init glfw3"};

            #ifdef SS_UTIL_GL_REQUIRES_ES
                glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
                glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
                glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
                window = glfwCreateWindow(w, h, title, nullptr, nullptr);
                limited_ = true;
            #else
                glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
                glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
                glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, true);
                constexpr int versions[][2] = {
                    {4, 6},
                    {4, 5},
                    {4, 3},
                    {},         // "limited" marker
                    {3, 3},
                };
                for (auto& ver: versions) {
                    if (ver[0] == 0 && ver[1] == 0) {
                        limited_ = true;
                        continue;
                    }
                    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, ver[0]);
                    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, ver[1]);
                    window = glfwCreateWindow(w, h, title, nullptr, nullptr);
                    if (window) break;
                }
            #endif

            if (!window) throw std::runtime_error{"failed to create OpenGL context"};
            glfwMakeContextCurrent(window);

            gl::init();
        }

        GL_Context::~GL_Context()
        {
            glfwTerminate();
        }
    }
}

