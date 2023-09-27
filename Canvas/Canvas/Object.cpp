#include "pch.h"
#include "Object.h"

namespace canvas
{
	Object::Object(D2D1_COLOR_F lineColor, D2D1_COLOR_F backgroundColor, float strokeWidth)
		: mRect({ NONE_POINT, NONE_POINT, NONE_POINT, NONE_POINT })
		, mLineColor(lineColor)
		, mBackgroundColor(backgroundColor)
		, mStrokeWidth(strokeWidth)
	{
	}

	Object::Object(D2D1_RECT_F rect, D2D1_COLOR_F lineColor, D2D1_COLOR_F backgroundColor, float strokeWidth)
		: mRect(rect)
		, mLineColor(lineColor)
		, mBackgroundColor(backgroundColor)
		, mStrokeWidth(strokeWidth)
	{
	}
}
