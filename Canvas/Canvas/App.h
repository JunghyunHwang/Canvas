#pragma once

#include "framework.h"

namespace canvas
{
	class App
	{
	public:
		static App* GetInstance();
		HRESULT Init(HWND, POINT);
		void Run();
		void Release();

		static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

	private:
		App();
		~App() = default;
		App(const App* other) = delete;
		App& operator=(const App* rhs) = delete;

		HRESULT createIndepentDeviceResource();
		HRESULT createDeviceResources();
		void discardDeviceResources();

		HRESULT onRender();
		HRESULT drawObjects();
		HRESULT drawNewObjectSize(D2D1_RECT_F objectRect);
		HRESULT drawDragSelection(D2D1_RECT_F selectedArea);

		void addObject();
		void moveSelectedObjects(int x, int y);
		Object* getObjectOnCursor(int x, int y) const;
		int getSelectedObjectsBoundary(D2D1_RECT_F& out);
		
	private:
		static App* mInstance;
		static bool mbLButtonDown;
		static D2D1_POINT_2F mStartPoint;
		static D2D1_POINT_2F mEndPoint;
		static eMouseMode mCurrMode;

		static std::unordered_set<Object*> mObjects; 
		static std::unordered_set<Object*> mSelectedObjects;
		static Object* mSelectedObjectsArea;

		HWND mHwnd;
		POINT mResolution;

		ID2D1Factory* mD2DFactory;
		ID2D1HwndRenderTarget* mRenderTarget;

		ID2D1SolidColorBrush* mObjectBrush;
		ID2D1SolidColorBrush* mLineBrush;
		ID2D1SolidColorBrush* mBackGroundBrush;
	};
}
