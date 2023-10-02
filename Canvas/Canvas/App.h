#pragma once

#include "Object.h"
#include "KeyManager.h"

namespace canvas
{
	class App final
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

	private:
		HRESULT createDeviceResources();
		void discardDeviceResources();

		HRESULT render();
		
		void addObject();
		void addCopiedObjectOnCursor(int x, int y);
		void duplicateSelectedObject();
		void removeSelectedObjects();
		void copySelectedObjects();
		void moveSelectedObjects(float x, float y);
		Object* getObjectOnCursor(float x, float y);
		void addObjectsInDraggingArea();
		void getSelectedObjectsBoundary(D2D1_RECT_F& out);
		void getResizeRect(D2D1_RECT_F& out);
		
		inline void setResizingRectsPoint();
		inline void setResizingRectsNone();
		
	private:
		static App* mInstance;
		static bool mbLButtonDown;
		static D2D1_POINT_2F mStartPoint;
		static D2D1_POINT_2F mEndPoint;
		static eMouseMode mCurrMode;

		static std::unordered_set<Object*> mObjects;
		static std::unordered_set<Object*> mSelectedObjects;
		static std::vector<ObjectInfo> mCopiedObjectInfo;

		static Object* mDragSelectionArea;
		static Object* mSelectedBoundary;
		static Object* mSelectedResizingRect;
		static eResizingDirection mResizingDirection;
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
		if (mSelectedBoundary->mRect.left == NONE_POINT)
		{
			return;
		}

		const D2D1_POINT_2F LEFT_TOP = { mSelectedBoundary->mRect.left, mSelectedBoundary->mRect.top };
		const D2D1_POINT_2F RIGHT_BOTTOM = { mSelectedBoundary->mRect.right, mSelectedBoundary->mRect.bottom };
		const D2D1_POINT_2F CENTER = mSelectedBoundary->GetCenter();
		
		mResizingRects[static_cast<int>(eResizingDirection::NorthWest)]->mRect = { 
			LEFT_TOP.x - RESIZING_RECT_SIZE,
			LEFT_TOP.y - RESIZING_RECT_SIZE,
			LEFT_TOP.x + RESIZING_RECT_SIZE,
			LEFT_TOP.y + RESIZING_RECT_SIZE
		};

		mResizingRects[static_cast<int>(eResizingDirection::NorthEast)]->mRect = {
			RIGHT_BOTTOM.x - RESIZING_RECT_SIZE,
			LEFT_TOP.y - RESIZING_RECT_SIZE,
			RIGHT_BOTTOM.x + RESIZING_RECT_SIZE,
			LEFT_TOP.y + RESIZING_RECT_SIZE
		};

		mResizingRects[static_cast<int>(eResizingDirection::SouthWest)]->mRect = {
			LEFT_TOP.x - RESIZING_RECT_SIZE,
			RIGHT_BOTTOM.y - RESIZING_RECT_SIZE,
			LEFT_TOP.x + RESIZING_RECT_SIZE,
			RIGHT_BOTTOM.y + RESIZING_RECT_SIZE
		};

		mResizingRects[static_cast<int>(eResizingDirection::SouthEast)]->mRect = {
			RIGHT_BOTTOM.x - RESIZING_RECT_SIZE,
			RIGHT_BOTTOM.y - RESIZING_RECT_SIZE,
			RIGHT_BOTTOM.x + RESIZING_RECT_SIZE,
			RIGHT_BOTTOM.y + RESIZING_RECT_SIZE
		};

		if (mSelectedObjects.size() == 1)
		{
			mResizingRects[static_cast<int>(eResizingDirection::North)]->mRect = {
				CENTER.x - RESIZING_RECT_SIZE,
				LEFT_TOP.y - RESIZING_RECT_SIZE,
				CENTER.x + RESIZING_RECT_SIZE,
				LEFT_TOP.y + RESIZING_RECT_SIZE
			};

			mResizingRects[static_cast<int>(eResizingDirection::West)]->mRect = {
				LEFT_TOP.x - RESIZING_RECT_SIZE,
				CENTER.y - RESIZING_RECT_SIZE,
				LEFT_TOP.x + RESIZING_RECT_SIZE,
				CENTER.y + RESIZING_RECT_SIZE
			};

			mResizingRects[static_cast<int>(eResizingDirection::East)]->mRect = {
				RIGHT_BOTTOM.x - RESIZING_RECT_SIZE,
				CENTER.y - RESIZING_RECT_SIZE,
				RIGHT_BOTTOM.x + RESIZING_RECT_SIZE,
				CENTER.y + RESIZING_RECT_SIZE
			};

			mResizingRects[static_cast<int>(eResizingDirection::South)]->mRect = {
				CENTER.x - RESIZING_RECT_SIZE,
				RIGHT_BOTTOM.y - RESIZING_RECT_SIZE,
				CENTER.x + RESIZING_RECT_SIZE,
				RIGHT_BOTTOM.y + RESIZING_RECT_SIZE
			};
		}
		else
		{
			SET_NONE_RECT(mResizingRects[static_cast<int>(eResizingDirection::North)]);
			SET_NONE_RECT(mResizingRects[static_cast<int>(eResizingDirection::West)]);
			SET_NONE_RECT(mResizingRects[static_cast<int>(eResizingDirection::East)]);
			SET_NONE_RECT(mResizingRects[static_cast<int>(eResizingDirection::South)]);
		}
	}

	inline void App::setResizingRectsNone()
	{
		for (auto obj : mResizingRects)
		{
			SET_NONE_RECT(obj);
		}
	}
}
