#include "App.h"

namespace canvas
{
	App* App::mInstance = nullptr;

	App::App()
		: mHwnd(nullptr)
		, mResolution{ 0, 0 }
		, mD2DFactory(nullptr)
		, mRenderTarget(nullptr)
	{
		mObjects.reserve(DEFAULT_OBJECT_CAPACITY);
	}

	App::~App()
	{
		SafeRelease(&mD2DFactory);
		SafeRelease(&mRenderTarget);

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
		}

		return hr;
	}

	void App::addObject(int x, int y)
	{
		mObjects.push_back(new Object(x, y, 100, 100, mRenderTarget));
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
							obj->mPos.x - obj->mScale.x / 2,
							obj->mPos.y - obj->mScale.y / 2,
							obj->mPos.x + obj->mScale.x / 2,
							obj->mPos.y + obj->mScale.y / 2
						),
						obj->mBrush
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
			mInstance->onRender();
			break;
		case WM_LBUTTONDOWN:
		{
			int mask = 0xFFFF;
			int x = lParam & mask;
			int y = (lParam >> 16) & mask;

			mInstance->addObject(x, y);
		}
		break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}

		return 0;
	}
}
