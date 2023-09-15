#include "Object.h"

namespace canvas
{
	uint32_t Object::mCount = 0;

	Object::Object(D2D1_POINT_2F leftTop, D2D1_POINT_2F rightBottom, D2D1::ColorF lineColor, D2D1::ColorF backgroundColor)
		: mLeftTop(leftTop)
		, mRightBottom(rightBottom)
		, mLineColor(lineColor)
		, mBackgroundColor(backgroundColor)
	{
		mID = ++mCount;
	}
}
