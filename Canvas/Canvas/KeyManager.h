#pragma once

namespace canvas
{
	enum class eKeyValue
	{
		Select,
		Rect,
		Backspace,
		C,
		V,
		D,
		Ctrl,

		Count
	};

	constexpr int TOTAL_KEY_COUNT = static_cast<int>(eKeyValue::Count);

	class KeyManager final
	{
	public:
		static KeyManager* GetInstance();
		
	public:
		void Update();
		bool IsKeyPressed(eKeyValue key);
		bool IsKeyPressed(int key);

	private:
		KeyManager();
		~KeyManager() = default;
		KeyManager(const KeyManager& other) = delete;
		KeyManager& operator=(const KeyManager& rhs) = delete;

	private:
		static KeyManager* mInstance;
		static int mVkArray[TOTAL_KEY_COUNT];
		bool mKeysPressed[TOTAL_KEY_COUNT];
	};
}
