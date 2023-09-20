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
		Object* getObjectOnCursor(float x, float y);
		int getSelectedObjectsBoundary(D2D1_RECT_F& out);

	private:
		static inline void setResizingRectsPoint();
		static inline void setResizingRectsNone();
		
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
		static Object* mSelectedResizingRect;
		static eResizingRect mSelectedResizingRectDirection;
		static Object* mNewObjectArea;
		static Object* mResizingRects[RESIZING_RECTS_COUNT];

		HWND mHwnd;
		POINT mResolution;

		ID2D1Factory* mD2DFactory;
		ID2D1HwndRenderTarget* mRenderTarget;
		ID2D1SolidColorBrush* mObjectBrush;
	};

	inline void App::setResizingRectsPoint()
	{
		if (mSelectedObjectsArea->mLeftTop.x == NONE_POINT)
		{
			return;
		}

		const D2D1_POINT_2F& LEFT_TOP = mSelectedObjectsArea->mLeftTop;
		const D2D1_POINT_2F& RIGHT_BOTTOM = mSelectedObjectsArea->mRightBottom;
		const float WIDTH = RIGHT_BOTTOM.x - LEFT_TOP.x;
		const float HEIGHT = RIGHT_BOTTOM.y - LEFT_TOP.y;
		const float CENTER_TO_WIDTH = RIGHT_BOTTOM.x - WIDTH / 2;
		const float CENTER_TO_HEGIHT = RIGHT_BOTTOM.y - HEIGHT / 2;
		int i = 0;

		mResizingRects[i]->mLeftTop = { LEFT_TOP.x - RESIZING_RECT_SIZE, LEFT_TOP.y - RESIZING_RECT_SIZE };
		mResizingRects[i++]->mRightBottom = { LEFT_TOP.x + RESIZING_RECT_SIZE, LEFT_TOP.y + RESIZING_RECT_SIZE };

		mResizingRects[i]->mLeftTop = { CENTER_TO_WIDTH - RESIZING_RECT_SIZE, LEFT_TOP.y - RESIZING_RECT_SIZE };
		mResizingRects[i++]->mRightBottom = { CENTER_TO_WIDTH + RESIZING_RECT_SIZE, LEFT_TOP.y + RESIZING_RECT_SIZE };

		mResizingRects[i]->mLeftTop = { RIGHT_BOTTOM.x - RESIZING_RECT_SIZE, LEFT_TOP.y - RESIZING_RECT_SIZE };
		mResizingRects[i++]->mRightBottom = { RIGHT_BOTTOM.x + RESIZING_RECT_SIZE, LEFT_TOP.y + RESIZING_RECT_SIZE };

		mResizingRects[i]->mLeftTop = { LEFT_TOP.x - RESIZING_RECT_SIZE, CENTER_TO_HEGIHT - RESIZING_RECT_SIZE };
		mResizingRects[i++]->mRightBottom = { LEFT_TOP.x + RESIZING_RECT_SIZE, CENTER_TO_HEGIHT + RESIZING_RECT_SIZE };

		mResizingRects[i]->mLeftTop = { RIGHT_BOTTOM.x - RESIZING_RECT_SIZE, CENTER_TO_HEGIHT - RESIZING_RECT_SIZE };
		mResizingRects[i++]->mRightBottom = { RIGHT_BOTTOM.x + RESIZING_RECT_SIZE, CENTER_TO_HEGIHT + RESIZING_RECT_SIZE };

		mResizingRects[i]->mLeftTop = { LEFT_TOP.x - RESIZING_RECT_SIZE, RIGHT_BOTTOM.y - RESIZING_RECT_SIZE };
		mResizingRects[i++]->mRightBottom = { LEFT_TOP.x + RESIZING_RECT_SIZE, RIGHT_BOTTOM.y + RESIZING_RECT_SIZE };

		mResizingRects[i]->mLeftTop = { CENTER_TO_WIDTH - RESIZING_RECT_SIZE, RIGHT_BOTTOM.y - RESIZING_RECT_SIZE };
		mResizingRects[i++]->mRightBottom = { CENTER_TO_WIDTH + RESIZING_RECT_SIZE, RIGHT_BOTTOM.y + RESIZING_RECT_SIZE };

		mResizingRects[i]->mLeftTop = { RIGHT_BOTTOM.x - RESIZING_RECT_SIZE, RIGHT_BOTTOM.y - RESIZING_RECT_SIZE };
		mResizingRects[i++]->mRightBottom = { RIGHT_BOTTOM.x + RESIZING_RECT_SIZE, RIGHT_BOTTOM.y + RESIZING_RECT_SIZE };

#ifdef _DEBUG
		if (i != RESIZING_RECTS_COUNT)
		{
			__debugbreak();
		}
#endif
	}

	inline void App::setResizingRectsNone()
	{
		for (auto obj : mResizingRects)
		{
			SET_NONE_RECT(obj);
		}
	}
}
