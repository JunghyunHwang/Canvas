#include "pch.h"
#include "Object.h"

namespace canvas
{
	Object::Object()
		: mLeftTop({ NONE_POINT, NONE_POINT })
		, mRightBottom({ NONE_POINT, NONE_POINT })
		, mLineColor(D2D1::ColorF::Black)
		, mBackgroundColor(D2D1::ColorF(0, 0, 0, 0.f))
	{
	}

	Object::Object(D2D1::ColorF lineColor, D2D1::ColorF backgroundColor)
		: mLeftTop({ NONE_POINT, NONE_POINT })
		, mRightBottom({ NONE_POINT, NONE_POINT })
		, mLineColor(lineColor)
		, mBackgroundColor(backgroundColor)
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
