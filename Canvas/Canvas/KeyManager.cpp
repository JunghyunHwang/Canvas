#include "pch.h"
#include "KeyManager.h"

namespace canvas
{
	KeyManager* KeyManager::mInstance = nullptr;

	int KeyManager::mVkArray[] = {
		'1',
		'2',
		VK_BACK,
		'C',
		'V',
		'D',
		VK_CONTROL
	};

	KeyManager::KeyManager()
	{
		for (size_t i = 0; i < TOTAL_KEY_COUNT; ++i)
		{
			mKeysPressed[i] = false;
		}
	}

	KeyManager* KeyManager::GetInstance()
	{
		if (mInstance == nullptr)
		{
			mInstance = new KeyManager();
		}

		return mInstance;
	}

	void KeyManager::Update()
	{
		for (size_t i = 0; i < TOTAL_KEY_COUNT; ++i)
		{
			if (PRESSED(GetAsyncKeyState(mVkArray[i])))
			{
				mKeysPressed[i] = true;
			}
			else
			{
				mKeysPressed[i] = false;
			}
		}
	}

	bool KeyManager::IsKeyPressed(eKeyValue key)
	{
		DEBUG_BREAK(static_cast<int>(key) > -1);
		DEBUG_BREAK(static_cast<int>(key) < static_cast<int>(eKeyValue::Count));

		return mKeysPressed[static_cast<int>(key)];
	}

	bool KeyManager::IsKeyPressed(int key)
	{
		DEBUG_BREAK(key > -1);
		DEBUG_BREAK(key < static_cast<int>(eKeyValue::Count));

		return mKeysPressed[key];
	}
}
