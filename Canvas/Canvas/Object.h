#pragma once

namespace canvas
{
	class App;

	class Object final
	{
		friend App;
	public:
		Object();
		Object(D2D1::ColorF lineColor, D2D1::ColorF backgroundColor);
		Object(D2D1_POINT_2F leftTop, D2D1_POINT_2F rigthBottom, D2D1::ColorF lineColor, D2D1::ColorF backgroundColor);
		~Object() = default;
		Object(const Object& other) = default;
		Object& operator=(const Object& rhs) = default;

		inline float GetWidth() const;
		inline float GetHeight() const;

		inline void SetLineColor(D2D1::ColorF color);
		inline void SetBackGroundColor(D2D1::ColorF color);
		inline void SetLeftTopPoint(D2D1_POINT_2F pos);
		inline void SetRightBottom(D2D1_POINT_2F pos);
		inline void SetRect(D2D1_RECT_F& rect);
		inline void Move(float x, float y);

	private:
		D2D1_POINT_2F mLeftTop;
		D2D1_POINT_2F mRightBottom;

		D2D1::ColorF mLineColor;
		D2D1::ColorF mBackgroundColor;
	};

	inline float Object::GetWidth() const
	{
		return mRightBottom.x - mLeftTop.x;
	}

	inline float Object::GetHeight() const
	{
		return mRightBottom.y - mLeftTop.y;
	}

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

	inline void Object::SetRect(D2D1_RECT_F& rect)
	{
		mLeftTop = { rect.left, rect.top };
		mRightBottom = { rect.right, rect.bottom };
	}

	inline void Object::Move(float x, float y)
	{
		mLeftTop.x += x;
		mLeftTop.y += y;
		mRightBottom.x += x;
		mRightBottom.y += y;
	}
}
