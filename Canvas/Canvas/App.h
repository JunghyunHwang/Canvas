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

		HRESULT onRender();
		HRESULT createIndepentDeviceResource();
		HRESULT createDeviceResources();
		void discardDeviceResources();

		void addObject(int x, int y);

	private:
		static App* mInstance;
		HWND mHwnd;
		POINT mResolution;
		std::vector<Object*> mObjects;

		ID2D1Factory* mD2DFactory;
		ID2D1HwndRenderTarget* mRenderTarget;
	};
}
