#pragma once

#include <cassert>
#include "framework.h"

namespace canvas
{
	struct vec2
	{
		float x;
		float y;
	};

	class App;

	class Object final
	{
		friend App;
	public:
		Object(int x, int y, int sX, int sY, ID2D1HwndRenderTarget* renderTarget);
		Object(float x, float y, float sX, float sY, ID2D1HwndRenderTarget* renderTarget);
		~Object();

	private:
		void setBrushColor(D2D1::ColorF::Enum color);

	private:
		vec2 mPos;
		vec2 mScale;
		ID2D1SolidColorBrush* mBrush;
	};
}
