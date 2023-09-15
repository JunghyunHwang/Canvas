#pragma once

#include "framework.h"

namespace canvas
{
	class App;

	class Object final
	{
		friend App;
	public:
		Object(D2D1_POINT_2F leftTop, D2D1_POINT_2F rigthBottom, D2D1::ColorF lineColor, D2D1::ColorF backgroundColor);
		~Object() = default;
		Object(const Object& other) = default;
		Object& operator=(const Object& rhs) = default;

		inline void SetLineColor(D2D1::ColorF color);
		inline void SetBackGroundColor(D2D1::ColorF color);
		inline void SetLeftTopPoint(D2D1_POINT_2F pos);
		inline void SetRightBottom(D2D1_POINT_2F pos);
		inline void Move(int x, int y);

	private:
		static uint32_t mCount;

		uint32_t mID;
		D2D1_POINT_2F mLeftTop;
		D2D1_POINT_2F mRightBottom;

		D2D1::ColorF mLineColor;
		D2D1::ColorF mBackgroundColor;
	};

	inline void Object::SetLineColor(D2D1::ColorF color)
	{
		mLineColor = color;
	}

	inline void Object::SetBackGroundColor(D2D1::ColorF color)
	{
		mBackgroundColor = color;
	}

	inline void Object::SetLeftTopPoint(D2D1_POINT_2F pos)
	{
		mLeftTop = pos;
	}

	inline void Object::SetRightBottom(D2D1_POINT_2F pos)
	{
		mRightBottom = pos;
	}

	inline void Object::Move(int x, int y)
	{
		mLeftTop.x += x;
		mLeftTop.y += y;
		mRightBottom.x += x;
		mRightBottom.y += y;
	}
}
