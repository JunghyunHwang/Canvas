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
		Object(D2D1::ColorF lineColor, D2D1::ColorF backgroundColor, float strokeWidth);
		Object(D2D1_RECT_F& rect, D2D1::ColorF lineColor, D2D1::ColorF backgroundColor);
		Object(D2D1_RECT_F& rect, D2D1::ColorF lineColor, D2D1::ColorF backgroundColor, float strokeWidth);

		~Object() = default;
		Object(const Object& other) = default;
		Object& operator=(const Object& rhs) = default;

		inline float GetWidth() const;
		inline float GetHeight() const;
		inline D2D1_POINT_2F GetCenter() const;

		inline void SetLeftTop(D2D1_POINT_2F& point);
		inline void SetRightBottom(D2D1_POINT_2F& point);
		inline void SetLineColor(D2D1::ColorF color);
		inline void SetBackGroundColor(D2D1::ColorF color);
		inline void SetRect(D2D1_RECT_F& rect);
		inline void SetStrokeWidth(float width);
		inline void Move(float x, float y);

	private:
		D2D1_RECT_F mRect;
		
		D2D1::ColorF mLineColor;
		D2D1::ColorF mBackgroundColor;
		float mStrokeWidth;
	};

	inline float Object::GetWidth() const
	{
		return mRect.right - mRect.left;
	}

	inline float Object::GetHeight() const
	{
		return mRect.bottom - mRect.top;
	}

	inline D2D1_POINT_2F Object::GetCenter() const
	{
		
		return { mRect.right - GetWidth() / 2, mRect.bottom - GetHeight() / 2 };
	}

	inline void Object::SetLeftTop(D2D1_POINT_2F& point)
	{
		mRect.left = point.x;
		mRect.top = point.y;
	}

	inline void Object::SetRightBottom(D2D1_POINT_2F& point)
	{
		mRect.right = point.x;
		mRect.bottom = point.y;
	}

	inline void Object::SetLineColor(D2D1::ColorF color)
	{
		mLineColor = color;
	}

	inline void Object::SetBackGroundColor(D2D1::ColorF color)
	{
		mBackgroundColor = color;
	}

	inline void Object::SetRect(D2D1_RECT_F& rect)
	{
		mRect = rect;
	}

	inline void Object::SetStrokeWidth(float width)
	{
		mStrokeWidth = width;
	}

	inline void Object::Move(float x, float y)
	{
		mRect.left += x;
		mRect.top += y;
		mRect.right += x;
		mRect.bottom += y;
	}
}
