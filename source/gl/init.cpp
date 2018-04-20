#include "../lib/gl.hpp"
#include "init.hpp"
#include <stdexcept>

#include "../util/compat/gl.macro.hpp"

namespace ss
{
    namespace gl
    {
        void init()
        {
            #ifndef SS_UTIL_GL_REQUIRES_ES
                ::glewExperimental = true;
                if (::glewInit() != GLEW_OK)
                    throw std::runtime_error{"Failed to load OpenGL Core Profile API with GLEW."};
            #endif
        }
    }
}

