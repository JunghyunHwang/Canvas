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
	Object* App::mSelectedObjectsArea = nullptr;
	Object* App::mNewObjectArea = nullptr;
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

		delete mSelectedObjectsArea;
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

		mDragSelectionArea = new Object(D2D1::ColorF(0.f, 0.f, 0.f, 0.3f), D2D1::ColorF(0x6495ED, 0.2f));
		mSelectedObjectsArea = new Object(D2D1::ColorF::Blue, D2D1::ColorF(0, 0, 0, 0.f));
		mNewObjectArea = new Object();

		RECT rt = { 0, 0, mResolution.x, mResolution.y };
		AdjustWindowRect(&rt, WS_OVERLAPPEDWINDOW, true);
		SetWindowPos(mHwnd, nullptr, 200, 200, rt.right - rt.left, rt.bottom - rt.top, 0);

		hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &mD2DFactory);

		if (SUCCEEDED(hr))
		{
			hr = createDeviceResources();
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

			SET_RECT_BY_OBJ_POINTER(rect, mSelectedObjectsArea);
			rect.left -= SELECTED_RECT_MARGIN;
			rect.top -= SELECTED_RECT_MARGIN;
			rect.right += SELECTED_RECT_MARGIN;
			rect.bottom += SELECTED_RECT_MARGIN;
			mObjectBrush->SetColor(mSelectedObjectsArea->mLineColor);
			mRenderTarget->DrawRectangle(rect, mObjectBrush);

			SET_RECT_BY_OBJ_POINTER(rect, mNewObjectArea);
			mObjectBrush->SetColor(mDragSelectionArea->mLineColor);
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
			if (ret == 0)
			{
				__debugbreak();
			}
#else
			mObjects.erase(obj);
#endif

			delete obj;
		}
	}

	Object* App::getObjectOnCursor(float x, float y) const
	{
		if (mCurrMode == eMouseMode::Selected)
		{
			if (x >= mSelectedObjectsArea->mLeftTop.x - OBJECT_MARGIN && x <= mSelectedObjectsArea->mRightBottom.x + OBJECT_MARGIN
				&& y >= mSelectedObjectsArea->mLeftTop.y - OBJECT_MARGIN && y <= mSelectedObjectsArea->mRightBottom.y + OBJECT_MARGIN)
			{
				return mSelectedObjectsArea;
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
			if (maxBottom != -1.f)
			{
				__debugbreak();
			}
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

	void App::moveSelectedObjects(float x, float y)
	{
		for (auto obj : mSelectedObjects)
		{
			obj->Move(x, y);
		}

		mSelectedObjectsArea->Move(x, y);
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
			if (mbLButtonDown)
			{
				__debugbreak();
			}

			mbLButtonDown = true;
			mStartPoint.x = LOWORD(lParam);
			mStartPoint.y = HIWORD(lParam);

			switch (mCurrMode)
			{
			case eMouseMode::Select:
			case eMouseMode::Selected:
			{
				Object* selected = mInstance->getObjectOnCursor(mStartPoint.x, mStartPoint.y);

				if (selected == mSelectedObjectsArea)
				{
					SetCursor(LoadCursor(nullptr, IDC_SIZEALL));
					break;
				}

				mSelectedObjects.clear();
				if (selected != nullptr)
				{
					SetCursor(LoadCursor(nullptr, IDC_SIZEALL));
					mSelectedObjects.insert(selected);

					mSelectedObjectsArea->SetLeftTopPoint({ selected->mLeftTop.x, selected->mLeftTop.y });
					mSelectedObjectsArea->SetRightBottom({ selected->mRightBottom.x, selected->mRightBottom.y });

					mCurrMode = eMouseMode::Selected;
				}
				else
				{
					SET_NONE_RECT(mSelectedObjectsArea);
					mCurrMode = eMouseMode::Select;
				}
			}
				break;
			case eMouseMode::Rect:
				SetCursor(LoadCursor(nullptr, IDC_CROSS));
				SET_NONE_RECT(mSelectedObjectsArea);
				break;
			default:
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

					mSelectedObjectsArea->SetLeftTopPoint({ selectedBoundary.left, selectedBoundary.top });
					mSelectedObjectsArea->SetRightBottom({ selectedBoundary.right, selectedBoundary.bottom });
					break;
				case eMouseMode::Selected:
					SetCursor(LoadCursor(nullptr, IDC_SIZEALL));

					mInstance->moveSelectedObjects(mEndPoint.x - mStartPoint.x, mEndPoint.y - mStartPoint.y);

					mStartPoint.x = mEndPoint.x;
					mStartPoint.y = mEndPoint.y;
					break;
				case eMouseMode::Rect:
					SetCursor(LoadCursor(nullptr, IDC_CROSS));
					mNewObjectArea->mLeftTop = mStartPoint;
					mNewObjectArea->mRightBottom = mEndPoint;
					break;
				default:
					__debugbreak();
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
						SetCursor(LoadCursor(nullptr, IDC_SIZEALL));
					}
					break;
				case eMouseMode::Rect:
					SetCursor(LoadCursor(nullptr, IDC_CROSS));
					break;
				default:
					__debugbreak();
					break;
				}
			}
			break;
		case WM_LBUTTONUP:
			if (!mbLButtonDown)
			{
				__debugbreak();
			}

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
			case eMouseMode::Rect:
				mInstance->addSelectedObject();

				SET_NONE_RECT(mNewObjectArea);

				mCurrMode = eMouseMode::Select;
				break;
			default:
				__debugbreak();
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

				SET_NONE_RECT(mSelectedObjectsArea);
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
