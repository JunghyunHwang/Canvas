#pragma once

#include <cassert>
#include "framework.h"

namespace canvas
{
	class App;

	class Object final
	{
		friend App;
	public:
		Object(D2D1_POINT_2F leftTop, D2D1_SIZE_F scale, D2D1::ColorF lineColor, D2D1::ColorF backgroundColor);
		~Object() = default;
		Object(const Object& other) = default;
		Object& operator=(const Object& rhs) = default;
		
	private:
		inline void setLineColor(D2D1::ColorF color);
		inline void setBackGroundColor(D2D1::ColorF color);

	private:
		D2D1_POINT_2F mLeftTop;
		D2D1_SIZE_F mScale;
		D2D1::ColorF mLineColor;
		D2D1::ColorF mBackgroundColor;
	};

	inline void Object::setLineColor(D2D1::ColorF color)
	{
		mLineColor = color;
	}

	inline void Object::setBackGroundColor(D2D1::ColorF color)
	{
		mBackgroundColor = color;
	}
}
