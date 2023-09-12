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
		Object(D2D1_POINT_2F center, D2D1_SIZE_F scale, ID2D1HwndRenderTarget* renderTarget);
		Object(int x, int y, int sX, int sY, ID2D1HwndRenderTarget* renderTarget);
		Object(float x, float y, float sX, float sY, ID2D1HwndRenderTarget* renderTarget);
		~Object();

	private:
		void setLineColor(D2D1::ColorF::Enum color);

	private:
		D2D1_POINT_2F mLeftTop;
		D2D1_SIZE_F mScale;
		ID2D1SolidColorBrush* mLine;
	};
}
