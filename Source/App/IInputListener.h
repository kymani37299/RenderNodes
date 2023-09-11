#pragma once

class IInputListener
{
public:
	virtual void OnKeyPressed(int key, int mods) {}
	virtual void OnKeyReleased(int key, int mods) {}
};