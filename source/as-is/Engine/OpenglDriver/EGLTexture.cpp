#include "EGLTexture.h"
#include "../../../util/compat/gl.macro.hpp"
#include "../../../util/unreachable.macro.hpp"

namespace
{
	void generate_mipmap(GLenum target, GLuint id)
	{
		glBindTexture(target, id);
		#ifdef SS_UTIL_GL_REQUIRES_ES
			glHint(GL_GENERATE_MIPMAP_HINT, GL_NICEST);
			glGenerateMipmap(target);
		#else
			// NOTE: There is no GL_GENERATE_MIPMAP_HINT in OpenGL 4.
			// DO NOT CALL glHint(GL_GENERATE_MIPMAP_HINT, GL_NICEST);
			if (target == GL_TEXTURE_CUBE_MAP) {
				glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
				glGenerateMipmap(target);
				glDisable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
			} else {
				glGenerateMipmap(target);
			}
		#endif
	}
}

EGLTexture::EGLTexture()
{
	texture_internal_format = Texture::RGBA8;
}

void EGLTexture::createInternalRes()
{
	if (m_valid)
		return;

	setFormat(texture_internal_format);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glGenTextures(1, &texture_id);

	texture_target = EGLUtil::Get(texture_type);
	if (MSAASampleNum > 1)
		texture_target = GL_TEXTURE_2D_MULTISAMPLE;
	glBindTexture(texture_target, texture_id);

	switch (mipmap_mode)
	{
	case Texture::TextureMipmap_Auto:
	case Texture::TextureMipmap_Manual:
		mipmap_level = CalcMipmapLevel(width, height);
		break;
	case Texture::TextureMipmap_None:
		mipmap_level = 1;
		break;
	default:
		break;
	}
	m_gl_texture_format_internal = getGLInternalFormat(texture_internal_format);
	if (texture_type == Texture_Cube) {
		createTextureCubeMap();
	}
	else if (texture_type == Texture_2D)
	{
		if (image)
			createTexture2DImage();
		else
			createTextureEmpty();
	}
	else if (texture_type == Texture2D_Array)
	{
		createTextureArrayEmpty();
	}

	GL_CHECK();
	m_valid = true;
}

void EGLTexture::useTexture()
{
	if (!m_valid)
		createInternalRes();

	glBindTexture(texture_target, texture_id);
}

void EGLTexture::releaseInternalRes()
{
	if (m_valid)
	{
		glDeleteTextures(1, &texture_id);
		texture_id = 0;
		m_valid = false;
	}
}

GLenum EGLTexture::getGLInternalFormat(Texture::TextureFormat format)
{
	switch (format)
	{
	case Texture::R32UI: return GL_R32UI;
	case Texture::RGBA8UI: return GL_RGBA8UI;
	case Texture::RGBA8: return GL_RGBA8;
	case Texture::DEPTH32: return GL_DEPTH_COMPONENT32F;
	case Texture::DEPTH: return GL_DEPTH_COMPONENT;
	case Texture::DEPTH16: return GL_DEPTH_COMPONENT16;
	case Texture::DEPTH24: return GL_DEPTH_COMPONENT24;
	case Texture::DEPTH24_STENCIL8: return GL_DEPTH24_STENCIL8;
	case Texture::RGBA32F: return GL_RGBA32F;
	case Texture::RGB32F: return GL_RGB32F;
	case Texture::RG32UI: return GL_RG32UI;
	case Texture::RGBA32UI: return GL_RGBA32UI;
	case Texture::RGBA16F: return GL_RGBA16F;
	case Texture::RGB16F: return GL_RGB16F;
	case Texture::RGBA16UI: return GL_RGBA16UI;
	case Texture::LUMINANCE: return GL_LUMINANCE;
	case Texture::RGB8: return GL_RGB8;
	case Texture::RGB10_A2: return GL_RGB10_A2;
	case Texture::R32F: return GL_R32F;
	case Texture::RG32F: return GL_RG32F;
	default: SS_UTIL_UNREACHABLE();
	}
}

void EGLTexture::GenerateMipmap()
{
	if (m_valid){
		generate_mipmap(texture_target, texture_id);
	}
	GL_CHECK();
}

void EGLTexture::getGLImageFormat(Image* im, GLenum& imageformat, GLenum& pixelformat)
{
	auto il = im->image_layout();
	pixelformat = is_f32(il) ? GL_FLOAT : GL_UNSIGNED_BYTE;
	switch (component_count(il)) {
		case 1: imageformat = GL_RED; break;
		case 2: imageformat = GL_RG; break;
		case 3: imageformat = GL_RGB; break;
		case 4: imageformat = GL_RGBA; break;
		default: SS_UTIL_UNREACHABLE();
	}
}

void EGLTexture::createTexture2DImage()
{
	glTexStorage2D(texture_target, mipmap_level, m_gl_texture_format_internal, width, height);
	GLenum gl_texture_format, gl_pixel_format;
	getGLImageFormat(image, gl_texture_format, gl_pixel_format);
	glTexSubImage2D(texture_target, 0, 0, 0, image->width(), image->height(), gl_texture_format, gl_pixel_format, image->data());
	if (mipmap_mode == TextureMipmap_Auto)
		generate_mipmap(texture_target, texture_id);
}

void EGLTexture::createTextureEmpty()
{
	#ifdef SS_UTIL_GL_REQUIRES_ES
		mipmap_level = 1;
		glTexStorage2D(texture_target, mipmap_level, m_gl_texture_format_internal, width, height);
	#else
		if (MSAASampleNum == 1)
			glTexStorage2D(texture_target, mipmap_level, m_gl_texture_format_internal, width, height);
		else
			glTexImage2DMultisample(texture_target, MSAASampleNum, m_gl_texture_format_internal, width, height, GL_TRUE);
	#endif
}

void EGLTexture::createTextureArrayEmpty()
{
	//glTexStorage3D(GL_TEXTURE_2D_ARRAY, LIGHT_TEXUTRE_MIPMAP_LEVELS, GL_DEPTH_COMPONENT32F, LIGHT_TEXTURE_SIZE, LIGHT_TEXTURE_SIZE, MAX_CAMERA_FRUSTUM_SPLIT_COUNT);
	glTexStorage3D(GL_TEXTURE_2D_ARRAY, mipmap_level, GL_DEPTH_COMPONENT32F, width, height, depth);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
}

void EGLTexture::createTextureCubeMap()
{

	glTexStorage2D(texture_target, mipmap_level, m_gl_texture_format_internal, width, height);
	if (image_cube.size() != 6) return;
	for (int i = 0; i < 6; i++){
		Image* image = image_cube[i];
		GLenum gl_texture_format, gl_pixel_format;
		getGLImageFormat(image, gl_texture_format, gl_pixel_format);
		glTexSubImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, 0, 0, image->width(), image->height(), gl_texture_format, gl_pixel_format, image->data());
		GL_CHECK();
	}

	//glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	if (mipmap_mode == TextureMipmap_Auto)
		generate_mipmap(texture_target, texture_id);
	GL_CHECK();
}

void EGLTexture::BlitMSAATexture(Texture* /*in*/, Texture* /*out*/)
{
	//out->createInternalRes();
	//EGLTexture* msaaTex = (EGLTexture*)in;
	//glReadBuffer(GL_COLOR_ATTACHMENT0);
	//glCopyTexSubImage2D()
	//glBindFramebuffer(GL_READ_FRAMEBUFFER, multisampledFBO);
	//glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	//glBlitFramebuffer(0, 0, width, height, 0, 0, width, height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
}

EGLTexture::~EGLTexture()
{
	releaseInternalRes();
}

