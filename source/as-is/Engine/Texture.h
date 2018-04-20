#pragma once
#include "../../resource/image.hpp"
#include "PreDefine.h"
//#include "SharedPointer.h"
#include <string>
#include <vector>
#include "Sampler.h"

using namespace std;

namespace HW
{
	class Texture
	{
	public:
		using Image = ss::resource::Image;
		using Image_Layout = ss::resource::Image_Layout;

		enum TextureFormat{
			LUMINANCE,
			RGB,
			RGBA,
			DEPTH,
			R8,
			RG8,
			RGB8,
			RGBA8,
			R16F,
			RG16F,
			RGB16F,
			RGBA16F,
			R32F,
			RG32F,
			RGB32F,
			RGBA32F,
			R11FG11FB10F,
			RGB10_A2,
			R8I,
			R16I,
			R32I,
			R8UI,
			R16UI,
			R32UI,
			RGBA16UI,
			RGBA8UI,
			RG32UI,
			RGBA32UI,
			DEPTH16,
			DEPTH24,
			DEPTH32,
			DEPTH24_STENCIL8,
		};
		enum SaveType {
			ST_FLOAT,
			ST_UNSIGNED_BYTE,
			ST_DEPTH,
			ST_Stencil
		};

		enum TextureType
		{
			Texture_2D,
			Texture2D_Array,
			Texture_3D,
			Texture_Cube
		};

		enum Cube_face
		{
			Cube_Back,
			Cube_Left,
			Cube_Right,
			Cube_Top,
			Cube_Bottom,
			Cube_FaceCount
		};

		enum TextureMipmap{
			TextureMipmap_Auto,
			TextureMipmap_Manual,
			TextureMipmap_None
		};
		std::string name;

		Sampler* texture_sampler;
		TextureType texture_type{Texture_2D};
		TextureFormat  texture_internal_format;


		unsigned int width=0;
		unsigned int height=0;
		unsigned int depth=1;

		TextureMipmap mipmap_mode = TextureMipmap_Auto;
		int mipmap_level=0;
		int MSAASampleNum=1;
		//image data
		Image* image=NULL;

		vector<Image*> image_cube;

		bool m_valid{false};

		Texture();

		virtual ~Texture() {}

		virtual void releaseInternalRes() {}

		virtual void createInternalRes() = 0;

		virtual void useTexture() = 0;

		virtual void setFormat(Texture::TextureFormat format);

		int CalcMipmapLevel(int width,int height);

		void setFormatFromImage(Image* im);
		virtual void GenerateMipmap(){

		}
		virtual void SaveTextureToImage(string /*name*/, SaveType /*t*/){}

		virtual void ReSize(int w, int h);
	};

	class  TextureFactory
	{
	public:
		virtual Texture * create() = 0;

	};
}

