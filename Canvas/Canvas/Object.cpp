#include "Object.h"

namespace canvas
{
	Object::Object(D2D1_POINT_2F leftTop, D2D1_SIZE_F scale, D2D1::ColorF lineColor, D2D1::ColorF backgroundColor)
		: mLeftTop(leftTop)
		, mScale(scale)
		, mLineColor(lineColor)
		, mBackgroundColor(backgroundColor)
	{
	}
}
