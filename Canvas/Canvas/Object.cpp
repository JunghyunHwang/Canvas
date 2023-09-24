#include "pch.h"
#include "Object.h"

namespace canvas
{
	Object::Object()
		: mRect({ NONE_POINT, NONE_POINT, NONE_POINT, NONE_POINT })
		, mLineColor(D2D1::ColorF::Black)
		, mBackgroundColor(D2D1::ColorF(0, 0, 0, 0.f))
		, mStrokeWidth(DEFAULT_STROKE_WIDTH)
	{
	}

	Object::Object(D2D1::ColorF lineColor, D2D1::ColorF backgroundColor)
		: mRect({ NONE_POINT, NONE_POINT, NONE_POINT, NONE_POINT })
		, mLineColor(lineColor)
		, mBackgroundColor(backgroundColor)
		, mStrokeWidth(DEFAULT_STROKE_WIDTH)
	{
	}

	Object::Object(D2D1::ColorF lineColor, D2D1::ColorF backgroundColor, float strokeWidth)
		: mRect({ NONE_POINT, NONE_POINT, NONE_POINT, NONE_POINT })
		, mLineColor(lineColor)
		, mBackgroundColor(backgroundColor)
		, mStrokeWidth(strokeWidth)
	{
	}

	Object::Object(D2D1_RECT_F& rect, D2D1::ColorF lineColor, D2D1::ColorF backgroundColor)
		: mRect(rect)
		, mLineColor(lineColor)
		, mBackgroundColor(backgroundColor)
		, mStrokeWidth(DEFAULT_STROKE_WIDTH)
	{
	}

	Object::Object(D2D1_RECT_F& rect, D2D1::ColorF lineColor, D2D1::ColorF backgroundColor, float strokeWidth)
		: mRect(rect)
		, mLineColor(lineColor)
		, mBackgroundColor(backgroundColor)
		, mStrokeWidth(strokeWidth)
	{
	}
}
