#pragma once

enum class KeyInputAction
{
	Pressed = 0,
	Released = 1,
	Repeat = 2,

	Undefined,
};

struct KeyInput
{
	int Key = 0;
	int Mods = 0;
	KeyInputAction Action = KeyInputAction::Undefined;
};

namespace std
{
	template <>
	struct hash<KeyInput>
	{
		size_t operator()(const KeyInput& k) const noexcept
		{
			size_t h1 = hash<int>{}(k.Key);
			size_t h2 = hash<int>{}(k.Mods);
			size_t h3 = hash<int>{}(static_cast<int>(k.Action));
			return h1 ^ (h2 << 1) ^ (h3 << 2);
		}
	};
}

inline bool operator==(const KeyInput& lhs, const KeyInput& rhs)
{
	return lhs.Key == rhs.Key &&
		lhs.Mods == rhs.Mods &&
		lhs.Action == rhs.Action;
}

class IInputListener
{
public:
	virtual void OnKeyInputEvent(const KeyInput& input) {}
};