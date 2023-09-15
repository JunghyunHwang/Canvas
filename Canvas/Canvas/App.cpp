#include "App.h"

namespace canvas
{
	App* App::mInstance = nullptr;
	bool App::mbLButtonDown = false;
	eMouseMode App::mCurrMode = eMouseMode::Select;
	D2D1_POINT_2F App::mStartPoint = { -1, -1 };
	D2D1_POINT_2F App::mEndPoint = { -1, -1 };

	Object* App::mSelectedObjectsArea = nullptr;
	std::unordered_set<Object*> App::mObjects;
	std::unordered_set<Object*> App::mSelectedObjects;

	App::App()
		: mHwnd(nullptr)
		, mResolution{ 0, 0 }
		, mD2DFactory(nullptr)
		, mRenderTarget(nullptr)
		, mObjectBrush(nullptr)
		, mLineBrush(nullptr)
		, mBackGroundBrush(nullptr)
	{
		mObjects.reserve(DEFAULT_OBJECT_CAPACITY);
		mSelectedObjects.reserve(DEFAULT_OBJECT_CAPACITY);
	}

	void App::discardDeviceResources()
	{
		SafeRelease(&mD2DFactory);
		SafeRelease(&mRenderTarget);

		SafeRelease(&mObjectBrush);
		SafeRelease(&mLineBrush);
		SafeRelease(&mBackGroundBrush);
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
		mSelectedObjectsArea = new Object({ -1, -1 }, { 0, 0 }, D2D1::ColorF::Blue, D2D1::ColorF(0, 0, 0, 0));

		RECT rt = { 0, 0, mResolution.x, mResolution.y };
		AdjustWindowRect(&rt, WS_OVERLAPPEDWINDOW, true);
		SetWindowPos(mHwnd, nullptr, 200, 200, rt.right - rt.left, rt.bottom - rt.top, 0);

		hr = createIndepentDeviceResource();

		if (SUCCEEDED(hr))
		{
			hr = createDeviceResources();
		}

		return hr;
	}

	HRESULT App::createIndepentDeviceResource()
	{
		HRESULT hr = S_OK;

		hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &mD2DFactory);

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
				mRenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black), &mLineBrush);
				mRenderTarget->CreateSolidColorBrush(D2D1::ColorF(0x6495ED, 0.3), &mBackGroundBrush);
			}
		}

		return hr;
	}

	HRESULT App::drawObjects()
	{
		mRenderTarget->BeginDraw();
		{
			mRenderTarget->SetTransform(D2D1::Matrix3x2F::Identity());
			mRenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::White));

			D2D1_RECT_F rect;

			for (auto obj : mObjects)
			{
				rect.left = obj->mLeftTop.x;
				rect.top = obj->mLeftTop.y;
				rect.right = obj->mRightBottom.x;
				rect.bottom = obj->mRightBottom.y;

				mObjectBrush->SetColor(obj->mBackgroundColor);
				mRenderTarget->FillRectangle(rect, mObjectBrush);

				mObjectBrush->SetColor(obj->mLineColor);
				mRenderTarget->DrawRectangle(rect, mObjectBrush);
			}

			if (mSelectedObjectsArea->mLeftTop.x >= 0)
			{
				rect.left = mSelectedObjectsArea->mLeftTop.x;
				rect.top = mSelectedObjectsArea->mLeftTop.y;
				rect.right = mSelectedObjectsArea->mRightBottom.x;
				rect.bottom = mSelectedObjectsArea->mRightBottom.y;

				mObjectBrush->SetColor(mSelectedObjectsArea->mLineColor);
				mRenderTarget->DrawRectangle(rect, mObjectBrush);
			}
		}
		return mRenderTarget->EndDraw();
	}

	HRESULT App::onRender()
	{
		HRESULT hr = createDeviceResources();

		if (SUCCEEDED(hr))
		{
			hr = drawObjects();

			if (hr == D2DERR_RECREATE_TARGET)
			{
				hr = S_OK;
				discardDeviceResources();
			}
		}

		return hr;
	}

	HRESULT App::drawNewObjectSize(D2D1_RECT_F objectRect)
	{
		HRESULT hr = createDeviceResources();

		if (SUCCEEDED(hr))
		{
			mRenderTarget->BeginDraw();
			{
				mRenderTarget->DrawRectangle(objectRect, mLineBrush);
			}
			mRenderTarget->EndDraw();

			drawObjects();

			if (hr == D2DERR_RECREATE_TARGET)
			{
				hr = S_OK;
				discardDeviceResources();
			}
		}

		return hr;
	}

	HRESULT App::drawDragSelection(D2D1_RECT_F selectedArea)
	{
		HRESULT hr = createDeviceResources();

		if (SUCCEEDED(hr))
		{
			mRenderTarget->BeginDraw();
			{
				mRenderTarget->FillRectangle(selectedArea, mBackGroundBrush);
				mRenderTarget->DrawRectangle(selectedArea, mLineBrush);
			}
			mRenderTarget->EndDraw();

			hr = drawObjects();

			if (hr == D2DERR_RECREATE_TARGET)
			{
				hr = S_OK;
				discardDeviceResources();
			}
		}

		return hr;
	}

	void App::addObject()
	{
		if (mStartPoint.x > mEndPoint.x)
		{
			int temp = mStartPoint.x;
			mStartPoint.x = mEndPoint.x;
			mEndPoint.x = temp;
		}

		if (mStartPoint.y > mEndPoint.y)
		{
			int temp = mStartPoint.y;
			mStartPoint.y = mEndPoint.y;
			mEndPoint.y = temp;
		}

		mObjects.insert(new Object(mStartPoint, mEndPoint, D2D1::ColorF::Black, D2D1::ColorF(0, 0, 0, 0)));
	}

	Object* App::getObjectOnCursor(int x, int y) const
	{
		Object* result = nullptr;

		if (mCurrMode == eMouseMode::Selected)
		{
			if (x >= mSelectedObjectsArea->mLeftTop.x - SELECT_MARGIN && x <= mSelectedObjectsArea->mRightBottom.x + SELECT_MARGIN
				&& y >= mSelectedObjectsArea->mLeftTop.y - SELECT_MARGIN && y <= mSelectedObjectsArea->mRightBottom.y + SELECT_MARGIN)
			{
				result = mSelectedObjectsArea;
				goto obj_return;
			}
		}

		for (auto obj : mObjects)
		{
			const float LEFT = obj->mLeftTop.x;
			const float RIGHT = obj->mRightBottom.x;
			const float TOP = obj->mLeftTop.y;
			const float BOTTOM = obj->mRightBottom.y;

			if (obj->mBackgroundColor.a > 0.0)
			{
				if (x >= obj->mLeftTop.x - SELECT_MARGIN && x <= obj->mRightBottom.x + SELECT_MARGIN
					&& y >= obj->mLeftTop.y - SELECT_MARGIN && y <= obj->mRightBottom.y + SELECT_MARGIN)
				{
					result = obj;
					break;
				}
			}
			else
			{
				if (y >= TOP - SELECT_MARGIN && y <= TOP + SELECT_MARGIN
					|| y >= BOTTOM - SELECT_MARGIN && y <= BOTTOM + SELECT_MARGIN)
				{
					if (x >= LEFT - SELECT_MARGIN && x <= RIGHT + SELECT_MARGIN)
					{
						result = obj;
						break;
					}
				}
				else if (x >= LEFT - SELECT_MARGIN && x <= LEFT + SELECT_MARGIN
					|| x >= RIGHT - SELECT_MARGIN && x <= RIGHT + SELECT_MARGIN)
				{
					if (y >= TOP - SELECT_MARGIN && y <= BOTTOM + SELECT_MARGIN)
					{
						result = obj;
						break;
					}
				}
			}
		}

	obj_return:
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

		float minLeft = mResolution.x + 1;
		float minTop = mResolution.y + 1;
		float maxRight = -1;
		float maxBottom = -1;
		int result = 0;
		
		for (auto obj : mObjects)
		{
			if (obj->mLeftTop.x >= dragSelectionArea.left && obj->mRightBottom.x <= dragSelectionArea.right
				&& obj->mLeftTop.y >= dragSelectionArea.top && obj->mRightBottom.y <= dragSelectionArea.bottom)
			{
				++result;
				mSelectedObjects.insert(obj);

				if (minLeft > obj->mLeftTop.x)
				{
					minLeft = obj->mLeftTop.x;
				}

				if (minTop > obj->mLeftTop.y)
				{
					minTop = obj->mLeftTop.y;
				}

				if (maxRight < obj->mRightBottom.x)
				{
					maxRight = obj->mRightBottom.x;
				}

				if (maxBottom < obj->mRightBottom.y)
				{
					maxBottom = obj->mRightBottom.y;
				}
			}
		}

		out.left = minLeft;
		out.top = minTop;
		out.right = maxRight;
		out.bottom = maxBottom;

		return result;
	}

	void App::moveSelectedObjects(int x, int y)
	{
		for (auto obj : mSelectedObjects)
		{
			obj->Move(x, y);
		}

		mSelectedObjectsArea->Move(x, y);

		if (drawObjects() == D2DERR_RECREATE_TARGET)
		{
			discardDeviceResources();
		}
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
		LRESULT result = 0;

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
					mInstance->drawDragSelection({ mStartPoint.x, mStartPoint.y, mEndPoint.x, mEndPoint.y });
					break;
				case eMouseMode::Rect:
					mInstance->drawNewObjectSize({ mStartPoint.x, mStartPoint.y, mEndPoint.x, mEndPoint.y });
					break;
				}
			}
			else
			{
				mInstance->onRender();
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
					break;
				}

				mSelectedObjects.clear();
				if (selected != nullptr)
				{
					SetCursor(LoadCursor(nullptr, IDC_SIZEALL));
					mSelectedObjects.insert(selected);

					mSelectedObjectsArea->SetLeftTopPoint({ selected->mLeftTop.x - SELECTED_RECT_MARGIN, selected->mLeftTop.y - SELECTED_RECT_MARGIN });
					mSelectedObjectsArea->SetRightBottom({ selected->mRightBottom.x + SELECTED_RECT_MARGIN, selected->mRightBottom.y + SELECTED_RECT_MARGIN });

					mCurrMode = eMouseMode::Selected;
				}
				else
				{
					mSelectedObjectsArea->SetLeftTopPoint({ -1, -1 });
					mSelectedObjectsArea->SetRightBottom({ 0, 0 });
					mCurrMode = eMouseMode::Select;
				}
			}
				break;
			case eMouseMode::Rect:
				SetCursor(LoadCursor(nullptr, IDC_CROSS));
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
					mInstance->drawDragSelection({ mStartPoint.x, mStartPoint.y, mEndPoint.x, mEndPoint.y });

					D2D1_RECT_F boundary;

					if (mInstance->getSelectedObjectsBoundary(boundary) > 0)
					{
						mSelectedObjectsArea->SetLeftTopPoint({ boundary.left - SELECTED_RECT_MARGIN, boundary.top - SELECTED_RECT_MARGIN });
						mSelectedObjectsArea->SetRightBottom({ boundary.right + SELECTED_RECT_MARGIN, boundary.bottom + SELECTED_RECT_MARGIN });
					}

					break;
				case eMouseMode::Selected:
					SetCursor(LoadCursor(nullptr, IDC_SIZEALL));
					mInstance->moveSelectedObjects(mEndPoint.x - mStartPoint.x, mEndPoint.y - mStartPoint.y);

					mStartPoint.x = mEndPoint.x;
					mStartPoint.y = mEndPoint.y;
					break;
				case eMouseMode::Rect:
					SetCursor(LoadCursor(nullptr, IDC_CROSS));
					mInstance->drawNewObjectSize({ mStartPoint.x, mStartPoint.y, mEndPoint.x, mEndPoint.y });
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
				break;
			case eMouseMode::Selected:
				break;
			case eMouseMode::Rect:
				mInstance->addObject();
				mInstance->mCurrMode = eMouseMode::Select;
				break;
			default:
				__debugbreak();
				break;
			}

			mbLButtonDown = false;
			mEndPoint.x = -1;
			mEndPoint.y = -1;
			break;
		case WM_KEYDOWN:
			if (GetAsyncKeyState('1') & 0x8000)
			{
				mCurrMode = eMouseMode::Select;
				SetCursor(LoadCursor(nullptr, IDC_ARROW));
			}
			else if (GetAsyncKeyState('2') & 0x8000)
			{
				mCurrMode = eMouseMode::Rect;
				SetCursor(LoadCursor(nullptr, IDC_CROSS));
			}
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}

		return 0;
	}
}
