#include "Object.h"

namespace canvas
{
	Object::Object()
		: mLeftTop({ -1, -1 })
		, mRightBottom({ 0, 0 })
		, mLineColor(D2D1::ColorF::Black)
		, mBackgroundColor(D2D1::ColorF(0, 0, 0, 0.f))
	{
	}

	Object::Object(D2D1_POINT_2F leftTop, D2D1_POINT_2F rightBottom, D2D1::ColorF lineColor, D2D1::ColorF backgroundColor)
		: mLeftTop(leftTop)
		, mRightBottom(rightBottom)
		, mLineColor(lineColor)
		, mBackgroundColor(backgroundColor)
	{
	}
}
