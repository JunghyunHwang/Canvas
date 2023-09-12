#include "Object.h"

namespace canvas
{
	Object::Object(D2D1_POINT_2F leftTop, D2D1_SIZE_F scale, ID2D1HwndRenderTarget* renderTarget)
		: mLeftTop(leftTop)
		, mScale(scale)
	{
		renderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black), &mLine);
	}

	Object::Object(int x, int y, int sX, int sY, ID2D1HwndRenderTarget* renderTarget)
		: Object(
			static_cast<float>(x),
			static_cast<float>(y),
			static_cast<float>(sX),
			static_cast<float>(sY),
			renderTarget
		)
	{
	}

	Object::Object(float x, float y, float sX, float sY, ID2D1HwndRenderTarget* renderTarget)
		: Object({ x, y }, { sX, sY }, renderTarget)
	{
	}

	Object::~Object()
	{
		mLine->Release();
	}


	void Object::setLineColor(D2D1::ColorF::Enum color)
	{
		assert(mLine != nullptr);
		mLine->SetColor(D2D1::ColorF(color));
	}
}
