#pragma once

#include "Object.h"

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

		HRESULT createDeviceResources();
		void discardDeviceResources();

		HRESULT render();

		void addSelectedObject();
		void removeSelectedObject();
		void moveSelectedObjects(float x, float y);
		Object* getObjectOnCursor(float x, float y) const;
		int getSelectedObjectsBoundary(D2D1_RECT_F& out);
		
	private:
		static App* mInstance;
		static bool mbLButtonDown;
		static D2D1_POINT_2F mStartPoint;
		static D2D1_POINT_2F mEndPoint;
		static eMouseMode mCurrMode;

		static std::unordered_set<Object*> mObjects; 
		static std::unordered_set<Object*> mSelectedObjects;

		static Object* mDragSelectionArea;
		static Object* mSelectedObjectsArea;
		static Object* mNewObjectArea;

		HWND mHwnd;
		POINT mResolution;

		ID2D1Factory* mD2DFactory;
		ID2D1HwndRenderTarget* mRenderTarget;
		ID2D1SolidColorBrush* mObjectBrush;
	};
}
