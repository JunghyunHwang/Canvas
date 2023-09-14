#include "App.h"

namespace canvas
{
	App* App::mInstance = nullptr;
	bool App::mbLButtonDown = false;
	eMouseMode App::mCurrMode = eMouseMode::Select;
	D2D1_POINT_2F App::mStartPoint = { -1, -1 };
	D2D1_POINT_2F App::mEndPoint = { -1, -1 };

	App::App()
		: mHwnd(nullptr)
		, mResolution{ 0, 0 }
		, mSelectedArea(nullptr)
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

		for (int i = 0; i < mObjects.size(); ++i)
		{
			delete mObjects[i];
		}

		delete mSelectedArea;
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
		mSelectedArea = new Object({ -1, -1 }, { 0, 0 }, D2D1::ColorF::Blue, D2D1::ColorF(0, 0, 0, 0));

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

			for (auto obj : mObjects)
			{
				mObjectBrush->SetColor(obj->mBackgroundColor);
				mRenderTarget->FillRectangle(obj->mRect, mObjectBrush);

				mObjectBrush->SetColor(obj->mLineColor);
				mRenderTarget->DrawRectangle(obj->mRect, mObjectBrush);
			}

			if (mSelectedArea->mLeftTop.x >= 0)
			{
				mObjectBrush->SetColor(mSelectedArea->mLineColor);
				mRenderTarget->DrawRectangle(mSelectedArea->mRect, mObjectBrush);
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

	HRESULT App::drawSelectedArea(D2D1_RECT_F selectedArea)
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
		int width = mEndPoint.x - mStartPoint.x;
		int height = mEndPoint.y - mStartPoint.y;

		if (width < 0)
		{
			mStartPoint.x = mEndPoint.x;
			width *= -1;
		}

		if (height < 0)
		{
			mStartPoint.y = mEndPoint.y;
			height *= -1;
		}

		assert(width >= 0);
		assert(height >= 0);

		D2D1_SIZE_F size = { width, height };

		mObjects.push_back(new Object(mStartPoint, size, D2D1::ColorF::Black, D2D1::ColorF(0, 0, 0, 0)));
	}

	Object* App::getObjectOnCursor(int x, int y) const
	{
		Object* result = nullptr;

		for (auto obj : mObjects)
		{
			const float LEFT = obj->mLeftTop.x;
			const float RIGHT = obj->mLeftTop.x + obj->mScale.width;
			const float TOP = obj->mLeftTop.y;
			const float BOTTOM = obj->mLeftTop.y + obj->mScale.height;

			if (obj->mBackgroundColor.a == 0.0)
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
			else
			{
				if (x >= obj->mLeftTop.x - SELECT_MARGIN
					&& x <= obj->mLeftTop.x + obj->mScale.width + SELECT_MARGIN
					&& y >= obj->mLeftTop.y - SELECT_MARGIN
					&& y <= obj->mLeftTop.y + obj->mScale.height + SELECT_MARGIN)
				{
					result = obj;
					break;
				}
			}
		}

		return result;
	}

	void App::setSelectedObjectsPoint(int x, int y)
	{
		for (auto obj : mSelectedObjects)
		{
			obj->setLeftTopPoint({ obj->mLeftTop.x + x, obj->mLeftTop.y + y });
		}

		mSelectedArea->setLeftTopPoint( { mSelectedArea->mLeftTop.x + x, mSelectedArea->mLeftTop.y + y });

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
					mInstance->drawSelectedArea({ mStartPoint.x, mStartPoint.y, mEndPoint.x, mEndPoint.y });
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
			assert(!mbLButtonDown);

			mbLButtonDown = true;
			mStartPoint.x = LOWORD(lParam);
			mStartPoint.y = HIWORD(lParam);

			switch (mCurrMode)
			{
			case eMouseMode::Select:
			case eMouseMode::Selected:
			{
				Object* selected = mInstance->getObjectOnCursor(mStartPoint.x, mStartPoint.y);

				mInstance->mSelectedObjects.clear();
				if (selected != nullptr)
				{
					mInstance->mSelectedObjects.push_back(selected);

					mInstance->mSelectedArea->setScale({ selected->mScale.width + SELECTED_RECT_MARGIN * 2, selected->mScale.height + SELECTED_RECT_MARGIN * 2 }); // Important order...
					mInstance->mSelectedArea->setLeftTopPoint({ selected->mLeftTop.x - SELECTED_RECT_MARGIN, selected->mLeftTop.y - SELECTED_RECT_MARGIN });

					mCurrMode = eMouseMode::Selected;
				}
				else
				{
					mInstance->mSelectedArea->setScale({ 0, 0 });
					mInstance->mSelectedArea->setLeftTopPoint({ -1, -1 });
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
					mInstance->drawSelectedArea({ mStartPoint.x, mStartPoint.y, mEndPoint.x, mEndPoint.y });
					break;
				case eMouseMode::Selected:
					mInstance->setSelectedObjectsPoint(mEndPoint.x - mStartPoint.x, mEndPoint.y - mStartPoint.y);

					mStartPoint.x = mEndPoint.x;
					mStartPoint.y = mEndPoint.y;
					break;
				case eMouseMode::Rect:
					SetCursor(LoadCursor(nullptr, IDC_CROSS));
					mInstance->drawNewObjectSize({ mStartPoint.x, mStartPoint.y, mEndPoint.x, mEndPoint.y });
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
						SetCursor(LoadCursor(nullptr, IDC_SIZEALL));
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
			assert(mbLButtonDown);

			switch (mCurrMode)
			{
			case eMouseMode::Select:
			case eMouseMode::Selected:
				break;
			case eMouseMode::Rect:
				mInstance->addObject();
				mInstance->mCurrMode = eMouseMode::Select;
				break;
			default:
				assert(false);
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
