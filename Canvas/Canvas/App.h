#pragma once

#include "framework.h"

namespace canvas
{
	template<typename Interface>
	inline void SafeRelease(Interface** interfaceToRelease)
	{
		if (*interfaceToRelease != nullptr)
		{
			(*interfaceToRelease)->Release();
			*interfaceToRelease = nullptr;
		}
	}

	class App
	{
	public:
		static App* GetInstance();
		HRESULT Init(HWND, POINT);
		void Run();

		static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

	private:
		App();
		~App();
		App(const App* other) = delete;
		App& operator=(const App* rhs) = delete;

		HRESULT createIndepentDeviceResource();
		HRESULT createDeviceResources();
		void discardDeviceResources();

		HRESULT onRender();
		HRESULT onRender(D2D1_RECT_F rect);
		void addObject();

	private:
		static App* mInstance;
		static bool mbLButtonDown;
		static bool mbDrawing;
		static D2D1_POINT_2F mLeftTop;
		static D2D1_POINT_2F mRightBottom;

		HWND mHwnd;
		POINT mResolution;
		std::vector<Object*> mObjects;

		ID2D1Factory* mD2DFactory;
		ID2D1HwndRenderTarget* mRenderTarget;
		ID2D1SolidColorBrush* mBlackBrush;
	};
}
