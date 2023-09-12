#include "App.h"

namespace canvas
{
	App* App::mInstance = nullptr;
	bool App::mbLButtonDown = false;
	bool App::mbDrawing = false;
	D2D1_POINT_2F App::mLeftTop = { -1, -1 };
	D2D1_POINT_2F App::mRightBottom = { -1, -1 };

	App::App()
		: mHwnd(nullptr)
		, mResolution{ 0, 0 }
		, mD2DFactory(nullptr)
		, mRenderTarget(nullptr)
		, mBlackBrush(nullptr)
	{
		mObjects.reserve(DEFAULT_OBJECT_CAPACITY);
	}

	App::~App()
	{
		SafeRelease(&mD2DFactory);
		SafeRelease(&mRenderTarget);
		SafeRelease(&mBlackBrush);

		delete mInstance;

		for (int i = 0; i < mObjects.size(); ++i)
		{
			delete mObjects[i];
		}
	}

	App* App::GetInstance()
	{
		if (mInstance == nullptr)
		{
			mInstance = new App();
		}

		return mInstance;
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

		hr = createIndepentDeviceResource();

		if (SUCCEEDED(hr))
		{
			hr = createDeviceResources();
		}

		return hr;
	}

	void App::discardDeviceResources()
	{
		SafeRelease(&mD2DFactory);
		SafeRelease(&mRenderTarget);
		SafeRelease(&mBlackBrush);
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
				mRenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black), &mBlackBrush);
			}
		}

		return hr;
	}

	HRESULT App::onRender()
	{
		HRESULT hr = S_OK;

		hr = createDeviceResources();

		if (SUCCEEDED(hr))
		{
			mRenderTarget->BeginDraw();
			{
				mRenderTarget->SetTransform(D2D1::Matrix3x2F::Identity());
				mRenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::White));

				for (auto obj : mObjects)
				{
					mRenderTarget->DrawRectangle(
						D2D1::RectF(
							obj->mLeftTop.x,
							obj->mLeftTop.y,
							obj->mLeftTop.x + obj->mScale.width,
							obj->mLeftTop.y + obj->mScale.height
						),
						obj->mLine
					);
				}
			}
			hr = mRenderTarget->EndDraw();

			if (hr == D2DERR_RECREATE_TARGET)
			{
				hr = S_OK;
				discardDeviceResources();
			}
		}

		return hr;
	}

	HRESULT App::onRender(D2D1_RECT_F rect)
	{
		HRESULT hr = S_OK;

		hr = createDeviceResources();

		if (SUCCEEDED(hr))
		{
			mRenderTarget->BeginDraw();
			{
				mRenderTarget->SetTransform(D2D1::Matrix3x2F::Identity());
				mRenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::White));

				mRenderTarget->DrawRectangle(rect, mBlackBrush);

				for (auto obj : mObjects)
				{
					mRenderTarget->DrawRectangle(
						D2D1::RectF(
							obj->mLeftTop.x,
							obj->mLeftTop.y,
							obj->mLeftTop.x + obj->mScale.width,
							obj->mLeftTop.y + obj->mScale.height
						),
						obj->mLine
					);
				}
			}
			hr = mRenderTarget->EndDraw();

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
		int width = mRightBottom.x - mLeftTop.x;
		int height = mRightBottom.y - mLeftTop.y;

		D2D1_SIZE_F size = { width, height };

		mObjects.push_back(new Object(mLeftTop, size, mRenderTarget));
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
			if (mbLButtonDown && mRightBottom.x >= 0)
			{
				mInstance->onRender({ mLeftTop.x, mLeftTop.y, mRightBottom.x, mRightBottom.y });
			}
			else
			{
				mInstance->onRender();
			}
			break;
		case WM_LBUTTONDOWN:
			assert(!mbLButtonDown);
			mbLButtonDown = true;

			mLeftTop.x = LOWORD(lParam);
			mLeftTop.y = HIWORD(lParam);
			break;
		case WM_MOUSEMOVE:
			if (mbLButtonDown)
			{
				mbDrawing = true;
				mRightBottom.x = LOWORD(lParam);
				mRightBottom.y = HIWORD(lParam);

				mInstance->onRender({ mLeftTop.x, mLeftTop.y, mRightBottom.x, mRightBottom.y });
			}

			break;
		case WM_LBUTTONUP:
			assert(mbLButtonDown);

			if (mbDrawing)
			{
				mInstance->addObject();
			}

			mbLButtonDown = false;
			mbDrawing = false;
			mRightBottom.x = -1;
			mRightBottom.y = -1;
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}

		return 0;
	}
}
