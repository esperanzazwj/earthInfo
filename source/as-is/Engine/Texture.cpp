#include "Texture.h"
#include "../../util/unreachable.macro.hpp"

HW::Texture::Texture()
{
	texture_sampler = SamplerManager::getInstance().get("RepeatLinearMipLinear");
}

int HW::Texture::CalcMipmapLevel(int width, int height)
{
	int res;
	if (width > height) res = width; else res = height;
	int level = 1;
	if (res > 1)
		level = int(log2(res))+1;
	return level;
}

void HW::Texture::setFormatFromImage(Image* im)
{
	switch (im->image_layout())
	{
		case Image_Layout::u8x1: texture_internal_format = R8; break;
		case Image_Layout::u8x2: texture_internal_format = RG8; break;
		case Image_Layout::u8x3: texture_internal_format = RGB8; break;
		case Image_Layout::u8x4: texture_internal_format = RGBA8; break;

		case Image_Layout::f32x1: texture_internal_format = R32F; break;
		case Image_Layout::f32x2: texture_internal_format = RG32F; break;
		case Image_Layout::f32x3: texture_internal_format = RGB32F; break;
		case Image_Layout::f32x4: texture_internal_format = RGBA32F; break;

		default: SS_UTIL_UNREACHABLE();
	}

	width = im->width();
	height = im->height();
}

void HW::Texture::ReSize(int w, int h)
{
	releaseInternalRes();
	width = w;
	height = h;
	createInternalRes();
}

void HW::Texture::setFormat(Texture::TextureFormat format)
{
	texture_internal_format = format;
}

