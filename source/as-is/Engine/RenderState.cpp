#include "RenderState.h"
#include <cassert>

namespace HW
{
	RasterState::RasterState(const RasterState& other)
	{
		this->mAntialiasedLineEnable = other.mAntialiasedLineEnable;
		this->mCullMode = other.mCullMode;
		this->mDepthBias = other.mDepthBias;
		this->mDepthBiasClamp = other.mDepthBiasClamp;
		this->mDepthClipEnable = other.mDepthClipEnable;
		this->mFillMode = other.mFillMode;
		this->mFrontCounterClockwise = other.mFrontCounterClockwise;
		this->mMultiSampleEnable = other.mMultiSampleEnable;
		this->mScissorEnable = other.mScissorEnable;
		this->mNumRects = other.mNumRects;
		this->mRects = NULL;
		if ( other.mNumRects != 0)
			this->mRects = new ScissorRect[other.mNumRects];
		for (unsigned int i=0; i<other.mNumRects; i++)
		{
			this->mRects[i] = other.mRects[i];
		}
		for(unsigned int i = 0; i< 4;i++)
			this->mColorWriteMask[i] = other.mColorWriteMask[i];
	}
	//////////////////////////////////////////////////////////////////////////
	RasterState& RasterState::operator=(const RasterState& other)
	{
		this->mAntialiasedLineEnable = other.mAntialiasedLineEnable;
		this->mCullMode = other.mCullMode;
		this->mDepthBias = other.mDepthBias;
		this->mDepthBiasClamp = other.mDepthBiasClamp;
		this->mDepthClipEnable = other.mDepthClipEnable;
		this->mFillMode = other.mFillMode;
		this->mFrontCounterClockwise = other.mFrontCounterClockwise;
		this->mMultiSampleEnable = other.mMultiSampleEnable;
		this->mScissorEnable = other.mScissorEnable;
		this->mNumRects = other.mNumRects;
		if (this->mRects == NULL)			
			this->mRects = new ScissorRect[other.mNumRects];
		for (unsigned int i=0; i<other.mNumRects; i++)
		{
			this->mRects[i] = other.mRects[i];
		}
		for (unsigned int i = 0; i< 4;i++)
		{
			this->mColorWriteMask[i] = other.mColorWriteMask[i];
		}
		return *this;
	}

	//////////////////////////////////////////////////////////////////////////
	bool RasterState::operator==(const RasterState& rhs)const
	{
		if(this->mAntialiasedLineEnable != rhs.mAntialiasedLineEnable)
			return false;
		if(this->mCullMode != rhs.mCullMode)
			return false;
		if(this->mDepthBias != rhs.mDepthBias)
			return false;
		if(this->mDepthBiasClamp != rhs.mDepthBiasClamp)
			return false;
		if(this->mDepthClipEnable != rhs.mDepthClipEnable)
			return false;
		if(this->mFillMode != rhs.mFillMode)
			return false;
		if(this->mFrontCounterClockwise != rhs.mFrontCounterClockwise)
			return false;
		if(this->mMultiSampleEnable != rhs.mMultiSampleEnable)
			return false;
		if(this->mNumRects != rhs.mNumRects)
			return false;
		if(memcmp(this->mRects,rhs.mRects,sizeof(ScissorRect)*this->mNumRects) != 0)
			return false;
		if (memcmp(this->mColorWriteMask,rhs.mColorWriteMask,sizeof(this->mColorWriteMask)) !=0)
		{
			return false;
		}
		return true;
	}
	//////////////////////////////////////////////////////////////////////////
	bool RasterState::operator!=(const RasterState &rhs)const
	{
		return !(*this==rhs);
	}

	//////////////////////////////////////////////////////////////////////////
	RasterState RasterState::getDefault()
	{
		RasterState state;
		state.mAntialiasedLineEnable = false;
		state.mCullMode = CULL_MODE_NONE;
		state.mDepthBias = 0;
		state.mDepthBiasClamp = 0;
		state.mDepthClipEnable = true;
		state.mFillMode = FM_SOLID;
		state.mFrontCounterClockwise = false;
		state.mMultiSampleEnable = false;
		state.mScissorEnable = false;
		state.mSlopScaledDepthBias = 0;
		state.mRects = NULL;
		for (unsigned int i = 0; i < 4 ;i++)
		{
			state.mColorWriteMask[i] = 1;
		}
		return state;
	}

	//////////////////////////////////////////////////////////////////////////
	void RasterState::setScissorRects(unsigned int numRect,ScissorRect rects[])
	{
		assert(numRect);
		if(!mNumRects)
		{
			mNumRects = numRect;
			mRects = new ScissorRect[numRect];
			for (unsigned int i=0; i<numRect; i++)
				mRects[i] = rects[i];
		}
		else if(this->mNumRects != numRect)
		{
			delete this->mRects;
			this->mRects = new ScissorRect[numRect];
			this->mNumRects = numRect;
			for (unsigned int i=0; i<numRect; i++)
				this->mRects[i] = rects[i];
		}
		else
		{
			for (unsigned int i=0; i<numRect; i++)
			{
				this->mRects[i] = rects[i];
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void RasterState::setScissorRectByIndex(unsigned int index,ScissorRect &rect)
	{
		assert(index < this->mNumRects);
		this->mRects[index] = rect;
	}


	BlendState::BlendState(const BlendState& other)
	{
		this->mAlphaToConverageEnable = other.mAlphaToConverageEnable;
		this->mIndependentBlendEnable = other.mIndependentBlendEnable;
		this->mBlendEnable = other.mBlendEnable;
		this->mBlendFunc = other.mBlendFunc;
		this->mBlendFuncAlpha = other.mBlendFuncAlpha;
		this->mDestBlend = other.mDestBlend;
		this->mDestBlendAlpla = other.mDestBlendAlpla;
		this->mSrcBlend = other.mSrcBlend;
		this->mSrcBlendAlpha = other.mSrcBlendAlpha;
		this->mWriteMask = other.mWriteMask;
		
		memcpy(this->mFactor,other.mFactor,sizeof(other.mFactor));
		this->mSampleMask = other.mSampleMask;
	}
	//////////////////////////////////////////////////////////////////////////
    BlendState::BlendState(bool blendEnable, BlendOperation op, BlendOperand src, BlendOperand dst)
    {
        mBlendEnable = blendEnable;
        mBlendFunc = op;
        mSrcBlend = src;
        mDestBlend = dst;
    }

	BlendState& BlendState::operator=(const BlendState &other)
	{
		this->mAlphaToConverageEnable = other.mAlphaToConverageEnable;
		this->mIndependentBlendEnable = other.mIndependentBlendEnable;
		
		this->mBlendEnable = other.mBlendEnable;
		this->mBlendFunc = other.mBlendFunc;
		this->mBlendFuncAlpha = other.mBlendFuncAlpha;
		this->mDestBlend = other.mDestBlend;
		this->mDestBlendAlpla = other.mDestBlendAlpla;
		this->mSrcBlend = other.mSrcBlend;
		this->mSrcBlendAlpha = other.mSrcBlendAlpha;
		this->mWriteMask = other.mWriteMask;

		memcpy(this->mFactor,other.mFactor,sizeof(other.mFactor));
		this->mSampleMask = other.mSampleMask;
		return *this;
	}
	//////////////////////////////////////////////////////////////////////////
	bool BlendState::operator==(const BlendState& rhs)const
	{
		if(memcmp(this,&rhs,sizeof(rhs)) != 0)
			return false;
		return true;
		if(this->mAlphaToConverageEnable != rhs.mAlphaToConverageEnable)
			return false;
		if(this->mIndependentBlendEnable != rhs.mIndependentBlendEnable)
			return false;
		if(memcmp(&this->mBlendEnable,&rhs.mBlendEnable,sizeof(rhs.mBlendEnable)) != 0)
			return false;
		if(memcmp(&this->mBlendFunc,&rhs.mBlendFunc,sizeof(rhs.mBlendFunc))!=0)
			return false;
		if(memcmp(&this->mBlendFuncAlpha,&rhs.mBlendFuncAlpha,sizeof(rhs.mBlendFuncAlpha))!= 0)
			return false;
		if(memcmp(&this->mDestBlend,&rhs.mDestBlend,sizeof(rhs.mDestBlend))!=0)
			return false;
		if(memcmp(&this->mDestBlendAlpla,&rhs.mDestBlendAlpla,sizeof(rhs.mDestBlendAlpla))!=0)
			return false;
		if(memcmp(&this->mSrcBlend,&rhs.mSrcBlend,sizeof(rhs.mSrcBlend))!=0)
			return false;
		if(memcmp(&this->mSrcBlendAlpha,&rhs.mSrcBlendAlpha,sizeof(rhs.mSrcBlendAlpha))!=0)
			return false;
		if(memcmp(&this->mWriteMask,&rhs.mWriteMask,sizeof(rhs.mWriteMask))!=0)
			return false;
		if(memcmp(&this->mFactor,&rhs.mFactor,sizeof(rhs.mFactor))!= 0)
			return false;
		if(this->mSampleMask != rhs.mSampleMask)
			return false;
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	BlendState BlendState::getDefault()
	{
		BlendState state;
		state.mAlphaToConverageEnable = false;
		state.mIndependentBlendEnable = false;
		//for (int i=0 ;i<8; i++)
		//{
			state.mBlendEnable = false;
			state.mSrcBlend = BLEND_ONE;
			state.mDestBlend = BLEND_ZERO;
			state.mBlendFunc = BLEND_OP_ADD;
			state.mSrcBlendAlpha = BLEND_ONE; 
			state.mDestBlendAlpla = BLEND_ZERO;
			state.mBlendFuncAlpha = BLEND_OP_ADD;
			state.mWriteMask = 1;
		//}
		return state;
	}

	//////////////////////////////////////////////////////////////////////////
	DepthStencilState DepthStencilState::getDefault()
	{
		DepthStencilState state;
		state.mDepthEnable = true;
		state.mDepthFunc = COMP_LESS_EQUAL;
		state.mStencilEnable = false;
		state.mStencilWriteMask = 0;// 
		state.mStencilReadMask = 0;
		state.mDepthWriteMask = true;
		state.mFrontStencilDepthFailFunc = STENCIL_OP_KEEP;
		state.mFrontStencilFunc = COMP_ALWAYS;
		state.mFrontStencilPassFunc = STENCIL_OP_KEEP;
		state.mFrontStencilFailFunc = STENCIL_OP_KEEP;
		state.mBackStencilDepthFailFunc = STENCIL_OP_KEEP;
		state.mBackStencilFailFunc = STENCIL_OP_KEEP;		
		state.mBackStencilPassFunc = STENCIL_OP_KEEP;
		state.mBackStencilFunc = COMP_ALWAYS;
		return state;
	}

	HW::ClearState ClearState::getDefault()
	{
		ClearState state;
		state.mclearcolorbuffer = true;
		state.mcleardepthbuffer = true;
		state.mclearstencilbuffer = true;
		state.mClearColor = Vector4(0, 0, 0, 0);
		state.mClearDepth = 1.0f;
		return state;
	}

}
