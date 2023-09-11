#include "Object.h"

namespace canvas
{
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
		: mPos{ x, y }
		, mScale{ sX, sY }
	{
		renderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black), &mBrush);
	}

	Object::~Object()
	{
		mBrush->Release();
	}


	void Object::setBrushColor(D2D1::ColorF::Enum color)
	{
		assert(mBrush != nullptr);
		mBrush->SetColor(D2D1::ColorF(color));
	}
}
