#pragma once
#include "../RenderSystemEnum.h"
#include "EGLTexture.h"
#include "OPENGL_Include.h"
#include <stdexcept>

using namespace HW;

#include "../../../util/unreachable.macro.hpp"

namespace ss
{
    namespace egl
    {
        std::string const& error_to_string(GLenum error);
    }
}

// FIXME: There are some unknown OpenGL errors scattering around.
//   Temporarily ignoring them for now.
//#define SS_GL_DEBUG
#ifdef SS_GL_DEBUG
    #undef SS_GL_DEBUG
    #include <stdexcept>
    #define GL_CHECK(...) do { \
        { __VA_ARGS__; } \
        auto glError = glGetError(); \
        if (glError != GL_NO_ERROR) { \
            auto& error = ::ss::egl::error_to_string(glError); \
            throw std::runtime_error{"OpenGL Error: " + error + "\n\n" + __FILE__ + ":" + std::to_string(__LINE__)}; \
        } \
    } while (0)
#else
    #define GL_CHECK(...) do { \
        { __VA_ARGS__; } \
    } while (0)
#endif

class EGLUtil final
{
public:
	static GLenum Get(ComparisonFunc func)
	{
		switch (func)
		{
		case COMP_NEVER:
			return GL_NEVER;
		case COMP_LESS:
			return GL_LESS;
		case COMP_EQUAL:
			return GL_EQUAL;
		case COMP_LESS_EQUAL:
			return GL_LEQUAL;
		case COMP_GREATER:
			return GL_GREATER;
		case COMP_NOT_EQUAL:
			return GL_NOTEQUAL;
		case COMP_GREATER_EQUAL:
			return GL_GEQUAL;
		case COMP_ALWAYS:
			return GL_ALWAYS;
		default:
			SS_UTIL_UNREACHABLE();
		}
	}

	static GLenum Get(BlendOperand bo)
	{
		switch (bo)
		{
		case BLEND_ZERO:
			return GL_ZERO;
		case BLEND_ONE:
			return GL_ONE;
		case BLEND_SRC_COLOR:
			return GL_SRC_COLOR;
		case BLEND_INV_SRC_COLOR:
			return GL_ONE_MINUS_SRC_COLOR;
		case BLEND_SRC_ALPHA:
			return GL_ALPHA;
		case BLEND_INV_SRC_ALPHA:
			return GL_ONE_MINUS_SRC_ALPHA;
		case BLEND_DEST_ALPHA:
			return GL_DST_ALPHA;
		case BLEND_INV_DEST_ALPHA:
			return GL_ONE_MINUS_DST_ALPHA;
		case BLEND_DEST_COLOR:
			return GL_DST_COLOR;
		case BLEND_INV_DEST_COLOR:
			return GL_ONE_MINUS_DST_COLOR;
		case BLEND_SRC_ALPHA_SAT:
			return GL_ZERO;
		case BLEND_BLEND_FACTOR:
			return GL_CONSTANT_COLOR;
		case BLEND_INV_BLEND_FACTOR:
			return GL_ONE_MINUS_CONSTANT_COLOR;
		default:
			SS_UTIL_UNREACHABLE();
		}
	}

	static GLenum Get(BlendOperation bo)
	{
		switch (bo)
		{
		case BLEND_OP_ADD:
			return GL_FUNC_ADD;
		case BLEND_OP_SUBTRACT:
			return GL_FUNC_SUBTRACT;
		case BLEND_OP_REV_SUBTRACT:
			return GL_FUNC_REVERSE_SUBTRACT;
		default:
			SS_UTIL_UNREACHABLE();
		}
	}

	static GLenum Get(StencilOperation so)
	{
		switch (so)
		{
		case STENCIL_OP_KEEP:
			return GL_KEEP;
		case STENCIL_OP_ZERO:
			return GL_ZERO;
		case STENCIL_OP_REPLACE:
			return GL_REPLACE;
		case STENCIL_OP_INCR_SAT:
			return GL_INCR_WRAP;
		case STENCIL_OP_DECR_SAT:
			return GL_DECR_WRAP;
		case STENCIL_OP_INVERT:
			return GL_INVERT;
		case STENCIL_OP_INCR:
			return GL_INCR;
		case STENCIL_OP_DECR:
			return GL_DECR;
		default:
			SS_UTIL_UNREACHABLE();
		}
	}

	static GLenum Get(Sampler::SamplerWrap warp)
	{
		switch (warp)
		{
		case Sampler::TextureWrap_Clamp:
			return GL_CLAMP_TO_EDGE;
		case Sampler::TextureWrap_Repeat:
			return GL_REPEAT;
		case Sampler::TextureWrap_Mirror:
			return GL_MIRRORED_REPEAT;
		default:
			SS_UTIL_UNREACHABLE();
		}
	}

	static GLenum Get(Sampler::TextureFilter filter)
	{
		switch (filter)
		{
		case Sampler::TextureFilter_Nearest:
			return GL_NEAREST;
		case Sampler::TextureFilter_Linear:
			return GL_LINEAR;
		case Sampler::TextureFilter_Nearest_Mipmap_Nearest:
			return GL_NEAREST_MIPMAP_NEAREST;
		case Sampler::TextureFilter_Linear_Mipmap_Nearest:
			return GL_LINEAR_MIPMAP_NEAREST;
		case Sampler::TextureFilter_Nearest_Mipmap_Linear:
			return GL_NEAREST_MIPMAP_LINEAR;
		case Sampler::TextureFilter_Linear_Mipmap_Linear:
			return GL_LINEAR_MIPMAP_LINEAR;
		default:
			return GL_LINEAR;
		}
	}

	static GLenum Get(Texture::TextureType texture_type)
	{
		switch (texture_type)
		{
		case HW::Texture::Texture_2D:
			return GL_TEXTURE_2D;
		case HW::Texture::Texture2D_Array:
			return GL_TEXTURE_2D_ARRAY;
		case HW::Texture::Texture_3D:
			return GL_TEXTURE_3D;
		case HW::Texture::Texture_Cube:
			return GL_TEXTURE_CUBE_MAP;
		default:
			SS_UTIL_UNREACHABLE();
		}
	}

	static GLenum Get(Texture::TextureFormat pixelformat)
	{
		switch (pixelformat)
		{
		case HW::Texture::LUMINANCE:
			return GL_LUMINANCE;
		case HW::Texture::RGBA8:
			return GL_RGBA;
		case HW::Texture::RGB8:
			return GL_RGB;
		case HW::Texture::DEPTH16:
			return GL_DEPTH_COMPONENT16;
		case HW::Texture::DEPTH32:
			return GL_DEPTH_COMPONENT32F;
		case HW::Texture::RGBA32F:
			return GL_RGBA32F;
		default:
			SS_UTIL_UNREACHABLE();
		}
	}

	static GLenum Get(BUFFER_STORE buffer_store)
	{
		switch (buffer_store)
		{
		case BS_STATIC:
			return GL_STATIC_DRAW;
			break;
		case BS_DYNAMIC:
			return GL_DYNAMIC_DRAW;
			break;
		case BS_STREAM:
			return GL_STREAM_DRAW;
			break;
		default:
			SS_UTIL_UNREACHABLE();
		}
	}
};

#include "../../../util/unreachable.undef.hpp"

