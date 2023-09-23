#include "pch.h"
#include "App.h"

namespace canvas
{
	App* App::mInstance = nullptr;
	bool App::mbLButtonDown = false;
	eMouseMode App::mCurrMode = eMouseMode::Select;
	D2D1_POINT_2F App::mStartPoint = { -1, -1 };
	D2D1_POINT_2F App::mEndPoint = { -1, -1 };

	Object* App::mDragSelectionArea = nullptr;
	Object* App::mSelectedBoundary = nullptr;
	Object* App::mSelectedResizingRect = nullptr;
	Object* App::mNewObjectArea = nullptr;
	Object* App::mResizingRects[RESIZING_RECTS_COUNT];
	eResizingDirection App::mResizingDirection = eResizingDirection::None;

	std::unordered_set<Object*> App::mObjects;
	std::unordered_set<Object*> App::mSelectedObjects;

	App::App()
		: mHwnd(nullptr)
		, mResolution{ 0, 0 }
		, mD2DFactory(nullptr)
		, mRenderTarget(nullptr)
		, mObjectBrush(nullptr)
	{
		mObjects.reserve(DEFAULT_OBJECT_CAPACITY);
		mSelectedObjects.reserve(DEFAULT_OBJECT_CAPACITY);
	}

	void App::discardDeviceResources()
	{
		SafeRelease(&mD2DFactory);
		SafeRelease(&mRenderTarget);

		SafeRelease(&mObjectBrush);
	}

	App* App::GetInstance()
	{
		if (mInstance == nullptr)
		{
			mInstance = new App();
		}

		return mInstance;
	}

	void App::Release()
	{
		discardDeviceResources();

		for (auto obj : mObjects)
		{
			delete obj;
		}

		for (auto obj : mResizingRects)
		{
			delete obj;
		}

		delete mSelectedBoundary;
		delete mDragSelectionArea;
		delete mNewObjectArea;
		delete mInstance;
	}

	HRESULT App::Init(HWND hWnd, POINT resolution)
	{
		if (mD2DFactory != nullptr)
		{
			return E_FAIL;
		}

		HRESULT hr = S_OK;
		mHwnd = hWnd;
		mResolution = resolution;

		RECT rt = { 0, 0, mResolution.x, mResolution.y };
		AdjustWindowRect(&rt, WS_OVERLAPPEDWINDOW, true);
		SetWindowPos(mHwnd, nullptr, 200, 200, rt.right - rt.left, rt.bottom - rt.top, 0);

		hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &mD2DFactory);

		if (SUCCEEDED(hr))
		{
			hr = createDeviceResources();
		}

		mDragSelectionArea = new Object(D2D1::ColorF(0.f, 0.f, 0.f, 0.3f), D2D1::ColorF(0x6495ED, 0.2f));
		mSelectedBoundary = new Object(D2D1::ColorF::Blue, D2D1::ColorF(0, 0, 0, 0.f));
		mNewObjectArea = new Object();

		for (size_t i = 0; i < RESIZING_RECTS_COUNT; ++i)
		{
			mResizingRects[i] = new Object(D2D1::ColorF::Blue, D2D1::ColorF(0xFFFFFF, 1.f));
		}

		return hr;
	}

	HRESULT App::createDeviceResources()
	{
		HRESULT hr = S_OK;

		if (mRenderTarget == nullptr)
		{
			RECT rt;
			GetClientRect(mHwnd, &rt);

			D2D1_SIZE_U size = D2D1::SizeU(rt.right - rt.left, rt.bottom - rt.top);

			hr = mD2DFactory->CreateHwndRenderTarget(
				D2D1::RenderTargetProperties(),
				D2D1::HwndRenderTargetProperties(mHwnd, size),
				&mRenderTarget
			);

			if (SUCCEEDED(hr))
			{
				mRenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black), &mObjectBrush);
			}
		}

		return hr;
	}

	HRESULT App::render()
	{
		mRenderTarget->BeginDraw();
		{
			mRenderTarget->SetTransform(D2D1::Matrix3x2F::Identity());
			mRenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::White));

			D2D1_RECT_F rect;

			for (auto obj : mObjects)
			{
				SET_RECT_BY_OBJ_POINTER(rect, obj);

				mObjectBrush->SetColor(obj->mBackgroundColor);
				mRenderTarget->FillRectangle(rect, mObjectBrush);

				mObjectBrush->SetColor(obj->mLineColor);
				mRenderTarget->DrawRectangle(rect, mObjectBrush);
			}

			SET_RECT_BY_OBJ_POINTER(rect, mDragSelectionArea);
			mObjectBrush->SetColor(mDragSelectionArea->mBackgroundColor);
			mRenderTarget->FillRectangle(rect, mObjectBrush);

			mObjectBrush->SetColor(mDragSelectionArea->mLineColor);
			mRenderTarget->DrawRectangle(rect, mObjectBrush);

			SET_RECT_BY_OBJ_POINTER(rect, mSelectedBoundary);
			mObjectBrush->SetColor(mSelectedBoundary->mLineColor);
			mRenderTarget->DrawRectangle(rect, mObjectBrush);

			if (mSelectedBoundary->mLeftTop.x == NONE_POINT)
			{
				setResizingRectsNone();
			}
			else
			{
				setResizingRectsPoint();

				for (auto obj : mResizingRects)
				{
					SET_RECT_BY_OBJ_POINTER(rect, obj);

					mObjectBrush->SetColor(obj->mBackgroundColor);
					mRenderTarget->FillRectangle(rect, mObjectBrush);

					mObjectBrush->SetColor(obj->mLineColor);
					mRenderTarget->DrawRectangle(rect, mObjectBrush);
				}
			}
			
			SET_RECT_BY_OBJ_POINTER(rect, mNewObjectArea);
			mObjectBrush->SetColor(mNewObjectArea->mLineColor);
			mRenderTarget->DrawRectangle(rect, mObjectBrush);
		}
		return mRenderTarget->EndDraw();
	}

	void App::addSelectedObject()
	{
		if (mStartPoint.x > mEndPoint.x)
		{
			const float temp = mStartPoint.x;
			mStartPoint.x = mEndPoint.x;
			mEndPoint.x = temp;
		}

		if (mStartPoint.y > mEndPoint.y)
		{
			const float temp = mStartPoint.y;
			mStartPoint.y = mEndPoint.y;
			mEndPoint.y = temp;
		}

		mObjects.insert(new Object(mStartPoint, mEndPoint, D2D1::ColorF::Black, D2D1::ColorF(0, 0, 0, 0)));
	}

	void App::removeSelectedObject()
	{
		for (auto obj : mSelectedObjects)
		{
#ifdef _DEBUG
			size_t ret = mObjects.erase(obj);
			DEBUG_BREAK(ret != 0);
#else
			mObjects.erase(obj);
#endif

			delete obj;
		}

		mSelectedObjects.clear();
	}

	Object* App::getObjectOnCursor(float x, float y)
	{
		if (mCurrMode == eMouseMode::Selected)
		{
			for (size_t i = 0; i < RESIZING_RECTS_COUNT; ++i)
			{
				if (x >= mResizingRects[i]->mLeftTop.x && x <= mResizingRects[i]->mRightBottom.x
					&& y >= mResizingRects[i]->mLeftTop.y && y <= mResizingRects[i]->mRightBottom.y)
				{
					mSelectedResizingRect = mResizingRects[i];
					mResizingDirection = static_cast<eResizingDirection>(i);

					return mResizingRects[i];
				}
			}

			mResizingDirection = eResizingDirection::None;
			
			if (x >= mSelectedBoundary->mLeftTop.x - OBJECT_MARGIN && x <= mSelectedBoundary->mRightBottom.x + OBJECT_MARGIN
				&& y >= mSelectedBoundary->mLeftTop.y - OBJECT_MARGIN && y <= mSelectedBoundary->mRightBottom.y + OBJECT_MARGIN)
			{
				return mSelectedBoundary;
			}
		}

		Object* result = nullptr;
		
		for (auto obj : mObjects)
		{
			const float LEFT = obj->mLeftTop.x;
			const float RIGHT = obj->mRightBottom.x;
			const float TOP = obj->mLeftTop.y;
			const float BOTTOM = obj->mRightBottom.y;

			if (obj->mBackgroundColor.a > 0.0)
			{
				if (x >= obj->mLeftTop.x - OBJECT_MARGIN && x <= obj->mRightBottom.x + OBJECT_MARGIN
					&& y >= obj->mLeftTop.y - OBJECT_MARGIN && y <= obj->mRightBottom.y + OBJECT_MARGIN)
				{
					result = obj;
					break;
				}
			}
			else
			{
				if (y >= TOP - OBJECT_MARGIN && y <= TOP + OBJECT_MARGIN
					|| y >= BOTTOM - OBJECT_MARGIN && y <= BOTTOM + OBJECT_MARGIN)
				{
					if (x >= LEFT - OBJECT_MARGIN && x <= RIGHT + OBJECT_MARGIN)
					{
						result = obj;
						break;
					}
				}
				else if (x >= LEFT - OBJECT_MARGIN && x <= LEFT + OBJECT_MARGIN
					|| x >= RIGHT - OBJECT_MARGIN && x <= RIGHT + OBJECT_MARGIN)
				{
					if (y >= TOP - OBJECT_MARGIN && y <= BOTTOM + OBJECT_MARGIN)
					{
						result = obj;
						break;
					}
				}
			}
		}

		return result;
	}

	int App::getSelectedObjectsBoundary(D2D1_RECT_F& out)
	{
		D2D1_RECT_F dragSelectionArea = { mStartPoint.x, mStartPoint.y, mEndPoint.x, mEndPoint.y };

		if (mStartPoint.x > mEndPoint.x)
		{
			dragSelectionArea.left = mEndPoint.x;
			dragSelectionArea.right = mStartPoint.x;
		}

		if (mStartPoint.y > mEndPoint.y)
		{
			dragSelectionArea.top = mEndPoint.y;
			dragSelectionArea.bottom = mStartPoint.y;
		}

		float minLeft = static_cast<float>(mResolution.x) + 1;
		float minTop = static_cast<float>(mResolution.y) + 1;
		float maxRight = -1.f;
		float maxBottom = -1.f;
		int result = 0;
		
		for (auto obj : mObjects)
		{
			if (obj->mLeftTop.x >= dragSelectionArea.left && obj->mRightBottom.x <= dragSelectionArea.right
				&& obj->mLeftTop.y >= dragSelectionArea.top && obj->mRightBottom.y <= dragSelectionArea.bottom)
			{
				auto ret = mSelectedObjects.insert(obj);

				if (ret.second)
				{
					++result;
				}

				minLeft = minLeft > obj->mLeftTop.x ? obj->mLeftTop.x : minLeft;
				minTop = minTop > obj->mLeftTop.y ? obj->mLeftTop.y : minTop;
				maxRight = maxRight < obj->mRightBottom.x ? obj->mRightBottom.x : maxRight;
				maxBottom = maxBottom < obj->mRightBottom.y ? obj->mRightBottom.y : maxBottom;
			}
			else
			{
				if (mSelectedObjects.find(obj) != mSelectedObjects.end())
				{
					mSelectedObjects.erase(obj);
				}
			}
		}

		if (maxRight == -1.f)
		{
#ifdef _DEBUG
			DEBUG_BREAK(maxBottom == -1.f);
#endif
			out.left = NONE_POINT;
			out.top = NONE_POINT;
			out.right = NONE_POINT;
			out.bottom = NONE_POINT;
		}
		else
		{
			out.left = minLeft;
			out.top = minTop;
			out.right = maxRight;
			out.bottom = maxBottom;
		}

		return result;
	}

	void App::getResizeRect(D2D1_RECT_F& out)
	{
		const float DIFF_X = mEndPoint.x - mStartPoint.x;
		const float DIFF_Y = mEndPoint.y - mStartPoint.y;

		switch (mResizingDirection)
		{
		case eResizingDirection::NorthWest:
			out = { DIFF_X, DIFF_Y, 0, 0 };
			SetCursor(LoadCursor(nullptr, IDC_SIZENWSE));
			break;
		case eResizingDirection::North:
			out = { 0, DIFF_Y, 0, 0 };
			SetCursor(LoadCursor(nullptr, IDC_SIZENS));
			break;
		case eResizingDirection::NorthEast:
			out = { 0, DIFF_Y, DIFF_X, 0 };
			SetCursor(LoadCursor(nullptr, IDC_SIZENESW));
			break;
		case eResizingDirection::West:
			out = { DIFF_X, 0, 0, 0 };
			SetCursor(LoadCursor(nullptr, IDC_SIZEWE));
			break;
		case eResizingDirection::East:
			out = { 0, 0, DIFF_X, 0 };
			SetCursor(LoadCursor(nullptr, IDC_SIZEWE));
			break;
		case eResizingDirection::SouthWest:
			out = { DIFF_X, 0, 0, DIFF_Y };
			SetCursor(LoadCursor(nullptr, IDC_SIZENESW));
			break;
		case eResizingDirection::South:
			out = { 0, 0, 0, DIFF_Y };
			SetCursor(LoadCursor(nullptr, IDC_SIZENS));
			break;
		case eResizingDirection::SouthEast:
			out = { 0, 0, DIFF_X, DIFF_Y };
			SetCursor(LoadCursor(nullptr, IDC_SIZENWSE));
			break;
		default:
			assert(false);
			break;
		}
	}

	void App::moveSelectedObjects(float x, float y)
	{
		for (auto obj : mSelectedObjects)
		{
			obj->Move(x, y);
		}

		mSelectedBoundary->Move(x, y);
	}

	void App::Run()
	{
		MSG msg;

		while (GetMessage(&msg, NULL, 0, 0))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	LRESULT App::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
	{
		switch (message)
		{
		case WM_DESTROY:
			PostQuitMessage(0);
			break;
		case WM_PAINT:
			if (mbLButtonDown && mEndPoint.x >= 0)
			{
				switch (mCurrMode)
				{
				case eMouseMode::Select:
					mDragSelectionArea->mLeftTop = mStartPoint;
					mDragSelectionArea->mRightBottom = mEndPoint;
					break;
				case eMouseMode::Rect:
					mNewObjectArea->mLeftTop = mStartPoint;
					mNewObjectArea->mRightBottom = mEndPoint;
					break;
				}
			}
			break;
		case WM_LBUTTONDOWN:
#ifdef _DEBUG
			DEBUG_BREAK(!mbLButtonDown);
#endif
			mbLButtonDown = true;
			mStartPoint.x = LOWORD(lParam);
			mStartPoint.y = HIWORD(lParam);

			switch (mCurrMode)
			{
			case eMouseMode::Select:
			case eMouseMode::Selected:
			{
				Object* selected = mInstance->getObjectOnCursor(mStartPoint.x, mStartPoint.y);

				if (selected == nullptr)
				{
					SET_NONE_RECT(mSelectedBoundary);
					mCurrMode = eMouseMode::Select;
					break;
				}
				else if (mResizingDirection != eResizingDirection::None)
				{
					mCurrMode = eMouseMode::Resize;

					switch (mResizingDirection)
					{
					case eResizingDirection::NorthWest:
					case eResizingDirection::SouthEast:
						SetCursor(LoadCursor(nullptr, IDC_SIZENWSE));
						break;
					case eResizingDirection::South:
					case eResizingDirection::North:
						SetCursor(LoadCursor(nullptr, IDC_SIZENS));
						break;
					case eResizingDirection::NorthEast:
					case eResizingDirection::SouthWest:
						SetCursor(LoadCursor(nullptr, IDC_SIZENESW));
						break;
					case eResizingDirection::West:
					case eResizingDirection::East:
						SetCursor(LoadCursor(nullptr, IDC_SIZEWE));
						break;
					case eResizingDirection::None:
						SetCursor(LoadCursor(nullptr, IDC_SIZEALL));
						break;
					default:
						assert(false);
						break;
					}

					break;
				}
				else if (selected == mSelectedBoundary)
				{
					SetCursor(LoadCursor(nullptr, IDC_SIZEALL));
					break;
				}

				mSelectedObjects.clear();

				SetCursor(LoadCursor(nullptr, IDC_SIZEALL));
				mSelectedObjects.insert(selected);

				D2D1_RECT_F rect;
				SET_RECT_BY_OBJ_POINTER(rect, selected);
				ADD_MARGIN_TO_RECT(rect, SELECTED_RECT_MARGIN);
				mSelectedBoundary->SetRect(rect);

				mCurrMode = eMouseMode::Selected;
			}
				break;
			case eMouseMode::Rect:
				SetCursor(LoadCursor(nullptr, IDC_CROSS));
				SET_NONE_RECT(mSelectedBoundary);
				break;
			default:
				assert(false);
				break;
			}
			break;
		case WM_MOUSEMOVE:
			if (mbLButtonDown)
			{
				mEndPoint.x = LOWORD(lParam);
				mEndPoint.y = HIWORD(lParam);

				switch (mCurrMode)
				{
				case eMouseMode::Select:
					mDragSelectionArea->mLeftTop = mStartPoint;
					mDragSelectionArea->mRightBottom = mEndPoint;

					D2D1_RECT_F selectedBoundary;
					mInstance->getSelectedObjectsBoundary(selectedBoundary);

					ADD_MARGIN_TO_RECT(selectedBoundary, SELECTED_RECT_MARGIN);
					mSelectedBoundary->SetRect(selectedBoundary);
					break;
				case eMouseMode::Selected:
					SetCursor(LoadCursor(nullptr, IDC_SIZEALL));

					mInstance->moveSelectedObjects(mEndPoint.x - mStartPoint.x, mEndPoint.y - mStartPoint.y);

					mStartPoint.x = mEndPoint.x;
					mStartPoint.y = mEndPoint.y;
					break;
				case eMouseMode::Resize:
				{
#ifdef _DEBUG
					DEBUG_BREAK(mSelectedResizingRect != nullptr);
					DEBUG_BREAK(mSelectedObjects.size() > 0);
#endif
					D2D1_RECT_F resize;

					if (mSelectedObjects.size() == 1)
					{
						mInstance->getResizeRect(resize);

						auto obj = mSelectedObjects.begin();
						(*obj)->mLeftTop.x += resize.left;
						(*obj)->mLeftTop.y += resize.top;
						(*obj)->mRightBottom.x += resize.right;
						(*obj)->mRightBottom.y += resize.bottom;
					}
					else
					{
						float diffX = mEndPoint.x - mStartPoint.x;
						float diffY = mEndPoint.y - mStartPoint.y;

						float oppositePointX;
						float oppositePointY;

						switch (mResizingDirection)
						{
						case eResizingDirection::NorthWest:
							SetCursor(LoadCursor(nullptr, IDC_SIZENWSE));
							if (diffY == 0)
							{
								if (diffX == -1)
								{
									diffY = -1;
								}
								else
								{
									diffX = 0;
								}
							}
							else if (diffX == 0)
							{
								if (diffY == -1)
								{
									diffX = -1;
								}
								else
								{
									diffY = 0;
								}
							}

							resize = { diffX, diffY, 0, 0 };
							oppositePointX = mSelectedBoundary->mRightBottom.x;
							oppositePointY = mSelectedBoundary->mRightBottom.y;
							break;
						case eResizingDirection::NorthEast:
							SetCursor(LoadCursor(nullptr, IDC_SIZENESW));
							if (diffY == 0)
							{
								if (diffX == 1)
								{
									diffY = -1;
								}
								else
								{
									diffX = 0;
								}
							}
							else if (diffX == 0)
							{
								if (diffY == -1)
								{
									diffX = 1;
								}
								else
								{
									diffY = 0;
								}
							}

							resize = { 0, diffY, diffX, 0 };

							oppositePointX = mSelectedBoundary->mLeftTop.x;
							oppositePointY = mSelectedBoundary->mRightBottom.y;
							break;
						case eResizingDirection::SouthWest:
							SetCursor(LoadCursor(nullptr, IDC_SIZENESW));
							if (diffY == 0)
							{
								if (diffX == -1)
								{
									diffY = 1;
								}
								else
								{
									diffX = 0;
								}
							}
							else if (diffX == 0)
							{
								if (diffY == 1)
								{
									diffX = -1;
								}
								else
								{
									diffY = 0;
								}
							}

							resize = { diffX, 0, 0, diffY };

							oppositePointX = mSelectedBoundary->mRightBottom.x;
							oppositePointY = mSelectedBoundary->mLeftTop.y;
							break;
						case eResizingDirection::SouthEast:
							SetCursor(LoadCursor(nullptr, IDC_SIZENWSE));
							if (diffY == 0)
							{
								if (diffX == 1)
								{
									diffY = 1;
								}
								else
								{
									diffX = 0;
								}
							}
							else if (diffX == 0)
							{
								if (diffY == 1)
								{
									diffX = 1;
								}
								else
								{
									diffY = 0;
								}
							}

							resize = { 0, 0, diffX, diffY };

							oppositePointX = mSelectedBoundary->mLeftTop.x;
							oppositePointY = mSelectedBoundary->mLeftTop.y;
							break;
						default:
							assert(false);
							break;
						}

						if (diffX == 0 && diffY == 0)
						{
							mEndPoint.x = mStartPoint.x;
							mEndPoint.y = mStartPoint.y;
							break;
						}

						const float boundaryWidth = mSelectedBoundary->GetWidth();
						const float bouddaryHeight = mSelectedBoundary->GetHeight();

						for (auto obj : mSelectedObjects)
						{
							obj->mLeftTop.x += abs(oppositePointX - obj->mLeftTop.x) / boundaryWidth * diffX;
							obj->mLeftTop.y += abs(oppositePointY - obj->mLeftTop.y) / bouddaryHeight * diffY;

							obj->mRightBottom.x += abs(oppositePointX - obj->mRightBottom.x) / boundaryWidth * diffX;
							obj->mRightBottom.y += abs(oppositePointY - obj->mRightBottom.y) / bouddaryHeight * diffY;
						}
					}

					mSelectedBoundary->mLeftTop.x += resize.left;
					mSelectedBoundary->mLeftTop.y += resize.top;
					mSelectedBoundary->mRightBottom.x += resize.right;
					mSelectedBoundary->mRightBottom.y += resize.bottom;

					mStartPoint.x = mEndPoint.x;
					mStartPoint.y = mEndPoint.y;
				}
					break;
				case eMouseMode::Rect:
					SetCursor(LoadCursor(nullptr, IDC_CROSS));
					mNewObjectArea->mLeftTop = mStartPoint;
					mNewObjectArea->mRightBottom = mEndPoint;
					break;
				default:
					assert(false);
					break;
				}
			}
			else
			{
				switch (mCurrMode)
				{
				case eMouseMode::Select:
				case eMouseMode::Selected:
					if (mInstance->getObjectOnCursor(LOWORD(lParam), HIWORD(lParam)))
					{
						switch (mResizingDirection)
						{
						case eResizingDirection::NorthWest:
						case eResizingDirection::SouthEast:
							SetCursor(LoadCursor(nullptr, IDC_SIZENWSE));
							break;
						case eResizingDirection::South:
						case eResizingDirection::North:
							SetCursor(LoadCursor(nullptr, IDC_SIZENS));
							break;
						case eResizingDirection::NorthEast:
						case eResizingDirection::SouthWest:
							SetCursor(LoadCursor(nullptr, IDC_SIZENESW));
							break;
						case eResizingDirection::West:
						case eResizingDirection::East:
							SetCursor(LoadCursor(nullptr, IDC_SIZEWE));
							break;
						case eResizingDirection::None:
							SetCursor(LoadCursor(nullptr, IDC_SIZEALL));
							break;
						default:
							assert(false);
							break;
						}
					}
					break;
				case eMouseMode::Rect:
					SetCursor(LoadCursor(nullptr, IDC_CROSS));
					break;
				default:
					assert(false);
					break;
				}
			}
			break;
		case WM_LBUTTONUP:
#ifdef _DEBUG
			DEBUG_BREAK(mbLButtonDown);
#endif
			switch (mCurrMode)
			{
			case eMouseMode::Select:
				if (mSelectedObjects.size() > 0)
				{
					mCurrMode = eMouseMode::Selected;
				}

				SET_NONE_RECT(mDragSelectionArea);
				break;
			case eMouseMode::Selected:
				if (mSelectedObjects.size() > 0)
				{
					SetCursor(LoadCursor(nullptr, IDC_SIZEALL));
					mCurrMode = eMouseMode::Selected;
				}
				break;
			case eMouseMode::Resize:
				mCurrMode = eMouseMode::Selected;
				mSelectedResizingRect = nullptr;
				mResizingDirection = eResizingDirection::None;
				break;
			case eMouseMode::Rect:
				mInstance->addSelectedObject();

				SET_NONE_RECT(mNewObjectArea);

				mCurrMode = eMouseMode::Select;
				break;
			default:
				assert(false);
				break;
			}

			mbLButtonDown = false;
			mEndPoint.x = NONE_POINT;
			mEndPoint.y = NONE_POINT;
			break;
		case WM_KEYDOWN:
			if (PRESSED(GetAsyncKeyState('1')))
			{
				mCurrMode = eMouseMode::Select;
				SetCursor(LoadCursor(nullptr, IDC_ARROW));
			}
			else if (PRESSED(GetAsyncKeyState('2')))
			{
				mCurrMode = eMouseMode::Rect;
				SetCursor(LoadCursor(nullptr, IDC_CROSS));
			}
			else if (PRESSED(GetAsyncKeyState('D')) || wParam == VK_BACK)
			{
				mInstance->removeSelectedObject();

				SET_NONE_RECT(mSelectedBoundary);
				mCurrMode = eMouseMode::Select;
			}
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}

		mInstance->render();

		return 0;
	}
}
