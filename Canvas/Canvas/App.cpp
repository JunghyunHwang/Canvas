#include "pch.h"
#include "App.h"

namespace canvas
{
	App* App::mInstance = nullptr;

	bool App::mbLButtonDown = false;
	D2D1_POINT_2F App::mStartPoint = { NONE_POINT, NONE_POINT };
	D2D1_POINT_2F App::mEndPoint = { NONE_POINT, NONE_POINT };

	Object* App::mDragSelectionArea = nullptr;
	Object* App::mSelectedBoundary = nullptr;
	Object* App::mSelectedResizingRect = nullptr;
	Object* App::mResizingRects[RESIZING_RECTS_COUNT];
	Object* App::mNewObjectArea = nullptr;

	eMouseMode App::mCurrMode = eMouseMode::Select;
	eResizingDirection App::mResizingDirection = eResizingDirection::None;

	std::unordered_set<Object*> App::mObjects;
	std::unordered_set<Object*> App::mSelectedObjects;
	std::vector<ObjectInfo> App::mCopiedObjectSizes;

	App::App()
		: mHwnd(nullptr)
		, mResolution{ 0, 0 }
		, mD2DFactory(nullptr)
		, mRenderTarget(nullptr)
		, mObjectBrush(nullptr)
	{
		mObjects.reserve(DEFAULT_OBJECT_CAPACITY);
		mSelectedObjects.reserve(DEFAULT_OBJECT_CAPACITY);
		mCopiedObjectSizes.reserve(DEFAULT_OBJECT_CAPACITY);
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

		mHwnd = hWnd;
		mResolution = resolution;

		RECT rt = { 0, 0, mResolution.x, mResolution.y };
		AdjustWindowRect(&rt, WS_OVERLAPPEDWINDOW, true);
		SetWindowPos(mHwnd, nullptr, 200, 200, rt.right - rt.left, rt.bottom - rt.top, 0);

		HRESULT hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &mD2DFactory);

		if (SUCCEEDED(hr))
		{
			hr = createDeviceResources();
		}

		mDragSelectionArea = new Object(D2D1::ColorF(0x000000, 0.3f), D2D1::ColorF(0x6495ED, 0.2f));
		mSelectedBoundary = new Object(D2D1::ColorF(0x0000FF, 1.f), D2D1::ColorF(0xFFFFFF, 0.f));
		mNewObjectArea = new Object(D2D1::ColorF(0x000000, 1.f), D2D1::ColorF(0xFFFFFF, 0.f));

		for (size_t i = 0; i < RESIZING_RECTS_COUNT; ++i)
		{
			mResizingRects[i] = new Object(D2D1::ColorF(0x0000FF, 1.f), D2D1::ColorF(0xFFFFFF, 1.0f));
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
				mRenderTarget->CreateSolidColorBrush(D2D1::ColorF(0x000000, 1.f), &mObjectBrush);
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

			for (auto obj : mObjects)
			{
				mObjectBrush->SetColor(obj->mBackgroundColor);
				mRenderTarget->FillRectangle(obj->mRect, mObjectBrush);

				mObjectBrush->SetColor(obj->mLineColor);
				mRenderTarget->DrawRectangle(obj->mRect, mObjectBrush, obj->mStrokeWidth);
			}

			mObjectBrush->SetColor(mDragSelectionArea->mBackgroundColor);
			mRenderTarget->FillRectangle(mDragSelectionArea->mRect, mObjectBrush);

			mObjectBrush->SetColor(mDragSelectionArea->mLineColor);
			mRenderTarget->DrawRectangle(mDragSelectionArea->mRect, mObjectBrush, mDragSelectionArea->mStrokeWidth);

			mObjectBrush->SetColor(mSelectedBoundary->mLineColor);
			mRenderTarget->DrawRectangle(mSelectedBoundary->mRect, mObjectBrush, mSelectedBoundary->mStrokeWidth);

			if (mSelectedBoundary->mRect.left == NONE_POINT)
			{
				setResizingRectsNone();
			}
			else
			{
				setResizingRectsPoint();

				for (auto obj : mResizingRects)
				{
					mObjectBrush->SetColor(obj->mBackgroundColor);
					mRenderTarget->FillRectangle(obj->mRect, mObjectBrush);

					mObjectBrush->SetColor(obj->mLineColor);
					mRenderTarget->DrawRectangle(obj->mRect, mObjectBrush, obj->mStrokeWidth);
				}
			}

			mObjectBrush->SetColor(mNewObjectArea->mLineColor);
			mRenderTarget->DrawRectangle(mNewObjectArea->mRect, mObjectBrush, mNewObjectArea->mStrokeWidth);
		}
		return mRenderTarget->EndDraw();
	}

	void App::addObject() 
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

		D2D1_RECT_F rect{ mStartPoint.x, mStartPoint.y, mEndPoint.x, mEndPoint.y };
		mObjects.insert(new Object(rect, D2D1::ColorF(0x000000, 1.f), D2D1::ColorF(0xFFFFFF, 0.f), 1.0f));
	}

	void App::addCopiedObjectOnCursor(int x, int y)
	{
		if (mCopiedObjectSizes.size() == 1)
		{
			D2D1_RECT_F rect {
				x - mCopiedObjectSizes[0].width / 2,
				y - mCopiedObjectSizes[0].height / 2,
				x + mCopiedObjectSizes[0].width / 2,
				y + mCopiedObjectSizes[0].height / 2
			};

			mObjects.insert(new Object(rect, mCopiedObjectSizes[0].lineColor, mCopiedObjectSizes[0].backgroundColor, mCopiedObjectSizes[0].strokeWidth));
		}
	}

	void App::duplicateSelectedObject()
	{
		if (mSelectedObjects.size() == 1)
		{
			auto obj = mSelectedObjects.begin();
			
			Object* newObj = new Object(**obj);
			newObj->Move(OBJECT_MARGIN, OBJECT_MARGIN);

			mObjects.insert(newObj);

			D2D1_RECT_F rect = newObj->mRect;
			ADD_MARGIN_TO_RECT(rect, SELECTED_RECT_MARGIN);
			mSelectedBoundary->SetRect(rect);

			mSelectedObjects.clear();
			mSelectedObjects.insert(newObj);
		}
	}

	void App::removeSelectedObjects()
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
			obj = nullptr;
		}

		mSelectedObjects.clear();
	}

	void App::copySelectedObjects()
	{
		mCopiedObjectSizes.clear();

		for (auto obj : mSelectedObjects)
		{
			mCopiedObjectSizes.push_back({ 
				obj->GetWidth(),
				obj->GetHeight(),
				obj->mLineColor,
				obj->mBackgroundColor,
				obj->mStrokeWidth
			});
		}
	}

	Object* App::getObjectOnCursor(float x, float y)
	{
		if (mCurrMode == eMouseMode::Selected)
		{
			for (size_t i = 0; i < RESIZING_RECTS_COUNT; ++i)
			{
				if (x >= mResizingRects[i]->mRect.left && x <= mResizingRects[i]->mRect.right
					&& y >= mResizingRects[i]->mRect.top && y <= mResizingRects[i]->mRect.bottom)
				{
					mSelectedResizingRect = mResizingRects[i];
					mResizingDirection = static_cast<eResizingDirection>(i);

					return mResizingRects[i];
				}
			}

			mResizingDirection = eResizingDirection::None;
			
			if (x >= mSelectedBoundary->mRect.left - OBJECT_MARGIN && x <= mSelectedBoundary->mRect.right + OBJECT_MARGIN
				&& y >= mSelectedBoundary->mRect.top - OBJECT_MARGIN && y <= mSelectedBoundary->mRect.bottom + OBJECT_MARGIN)
			{
				return mSelectedBoundary;
			}
		}

		Object* result = nullptr;
		
		for (auto obj : mObjects)
		{
			const float LEFT = obj->mRect.left;
			const float TOP = obj->mRect.top;
			const float RIGHT = obj->mRect.right;
			const float BOTTOM = obj->mRect.bottom;

			if (obj->mBackgroundColor.a > 0.0)
			{
				if (x >= obj->mRect.left - OBJECT_MARGIN && x <= obj->mRect.right + OBJECT_MARGIN
					&& y >= obj->mRect.top - OBJECT_MARGIN && y <= obj->mRect.bottom + OBJECT_MARGIN)
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
			if (obj->mRect.left >= dragSelectionArea.left && obj->mRect.right <= dragSelectionArea.right
				&& obj->mRect.top >= dragSelectionArea.top && obj->mRect.bottom <= dragSelectionArea.bottom)
			{
				auto ret = mSelectedObjects.insert(obj);

				if (ret.second)
				{
					++result;
				}

				minLeft = minLeft > obj->mRect.left ? obj->mRect.left : minLeft;
				minTop = minTop > obj->mRect.top ? obj->mRect.top : minTop;
				maxRight = maxRight < obj->mRect.right ? obj->mRect.right: maxRight;
				maxBottom = maxBottom < obj->mRect.bottom ? obj->mRect.bottom : maxBottom;
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
			DEBUG_BREAK(maxBottom == -1.f);

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
			DEBUG_BREAK(false);
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
		static KeyManager* keyManager = KeyManager::GetInstance();

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
					mDragSelectionArea->SetLeftTop(mStartPoint);
					mDragSelectionArea->SetRightBottom(mEndPoint);
					break;
				case eMouseMode::Rect:
					mNewObjectArea->SetLeftTop(mStartPoint);
					mNewObjectArea->SetRightBottom(mEndPoint);
					break;
				}
			}
			break;
		case WM_LBUTTONDOWN:
			DEBUG_BREAK(!mbLButtonDown);
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
						DEBUG_BREAK(false);
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

				D2D1_RECT_F rect = selected->mRect;
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
				DEBUG_BREAK(false);
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
					mDragSelectionArea->SetLeftTop(mStartPoint);
					mDragSelectionArea->SetRightBottom(mEndPoint);

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
					DEBUG_BREAK(mSelectedResizingRect != nullptr);
					DEBUG_BREAK(mSelectedObjects.size() > 0);
					D2D1_RECT_F resize;

					if (mSelectedObjects.size() == 1)
					{
						mInstance->getResizeRect(resize);

						auto obj = mSelectedObjects.begin();
						(*obj)->mRect.left += resize.left;
						(*obj)->mRect.top += resize.top;
						(*obj)->mRect.right += resize.right;
						(*obj)->mRect.bottom += resize.bottom;
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
							oppositePointX = mSelectedBoundary->mRect.right;
							oppositePointY = mSelectedBoundary->mRect.bottom;
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

							oppositePointX = mSelectedBoundary->mRect.left;
							oppositePointY = mSelectedBoundary->mRect.bottom;
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

							oppositePointX = mSelectedBoundary->mRect.right;
							oppositePointY = mSelectedBoundary->mRect.top;
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

							oppositePointX = mSelectedBoundary->mRect.left;
							oppositePointY = mSelectedBoundary->mRect.top;
							break;
						default:
							DEBUG_BREAK(false);
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
							obj->mRect.left += abs(oppositePointX - obj->mRect.left) / boundaryWidth * diffX;
							obj->mRect.top += abs(oppositePointY - obj->mRect.top) / bouddaryHeight * diffY;

							obj->mRect.right += abs(oppositePointX - obj->mRect.right) / boundaryWidth * diffX;
							obj->mRect.bottom += abs(oppositePointY - obj->mRect.bottom) / bouddaryHeight * diffY;
						}
					}

					mSelectedBoundary->mRect.left += resize.left;
					mSelectedBoundary->mRect.top += resize.top;
					mSelectedBoundary->mRect.right += resize.right;
					mSelectedBoundary->mRect.bottom += resize.bottom;

					mStartPoint.x = mEndPoint.x;
					mStartPoint.y = mEndPoint.y;
				}
					break;
				case eMouseMode::Rect:
					SetCursor(LoadCursor(nullptr, IDC_CROSS));
					mNewObjectArea->SetLeftTop(mStartPoint);
					mNewObjectArea->SetRightBottom(mEndPoint);
					break;
				default:
					DEBUG_BREAK(false);
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
							DEBUG_BREAK(false);
							break;
						}
					}
					break;
				case eMouseMode::Rect:
					SetCursor(LoadCursor(nullptr, IDC_CROSS));
					break;
				default:
					DEBUG_BREAK(false);
					break;
				}
			}
			break;
		case WM_LBUTTONUP:
			DEBUG_BREAK(mbLButtonDown);

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
				mInstance->addObject();

				SET_NONE_RECT(mNewObjectArea);

				mCurrMode = eMouseMode::Select;
				break;
			default:
				DEBUG_BREAK(false);
				break;
			}

			mbLButtonDown = false;
			mEndPoint.x = NONE_POINT;
			mEndPoint.y = NONE_POINT;
			break;
		case WM_KEYDOWN:
			keyManager->Update();

			if (keyManager->IsKeyPressed(eKeyValue::Select))
			{
				mCurrMode = eMouseMode::Select;
				SetCursor(LoadCursor(nullptr, IDC_ARROW));
			}
			else if (keyManager->IsKeyPressed(eKeyValue::Rect))
			{
				mCurrMode = eMouseMode::Rect;
				SetCursor(LoadCursor(nullptr, IDC_CROSS));
			}
			else if (keyManager->IsKeyPressed(eKeyValue::Backspace))
			{
				mInstance->removeSelectedObjects();

				SET_NONE_RECT(mSelectedBoundary);
				mCurrMode = eMouseMode::Select;

				goto call_render;
			}
			else if (keyManager->IsKeyPressed(eKeyValue::C) && keyManager->IsKeyPressed(eKeyValue::Ctrl))
			{
				mInstance->copySelectedObjects();
			}
			else if (keyManager->IsKeyPressed(eKeyValue::V) && keyManager->IsKeyPressed(eKeyValue::Ctrl))
			{
				POINT cursorPoint = { 0, 0 };
				GetCursorPos(&cursorPoint);
				ScreenToClient(mInstance->mHwnd, &cursorPoint);

				mInstance->addCopiedObjectOnCursor(cursorPoint.x, cursorPoint.y);

				goto call_render;
			}
			else if (keyManager->IsKeyPressed(eKeyValue::D) && keyManager->IsKeyPressed(eKeyValue::Ctrl))
			{
				mInstance->duplicateSelectedObject();
				goto call_render;
			}

			goto no_render;
		case WM_KEYUP:
			keyManager->Update();

			goto no_render;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}

	call_render:
		mInstance->render();

	no_render:
		return 0;
	}
}
