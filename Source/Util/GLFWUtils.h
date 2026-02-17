#pragma once

#include <vector>

#include "../Common.h"

namespace GLFWUtils
{
	inline std::string KeyToUIKey(int key) {
		// Letters
		if (key >= GLFW_KEY_A && key <= GLFW_KEY_Z) {
			return "Key" + std::string(1, 'A' + (key - GLFW_KEY_A));
		}

		// Numbers
		if (key >= GLFW_KEY_0 && key <= GLFW_KEY_9) {
			return "Digit" + std::string(1, '0' + (key - GLFW_KEY_0));
		}

		// Function keys
		if (key >= GLFW_KEY_F1 && key <= GLFW_KEY_F25) {
			return "F" + std::to_string(key - GLFW_KEY_F1 + 1);
		}

		// Special keys
		switch (key) {
		case GLFW_KEY_SPACE: return "Space";
		case GLFW_KEY_APOSTROPHE: return "Quote";
		case GLFW_KEY_COMMA: return "Comma";
		case GLFW_KEY_MINUS: return "Minus";
		case GLFW_KEY_PERIOD: return "Period";
		case GLFW_KEY_SLASH: return "Slash";
		case GLFW_KEY_SEMICOLON: return "Semicolon";
		case GLFW_KEY_EQUAL: return "Equal";
		case GLFW_KEY_LEFT_BRACKET: return "BracketLeft";
		case GLFW_KEY_BACKSLASH: return "Backslash";
		case GLFW_KEY_RIGHT_BRACKET: return "BracketRight";
		case GLFW_KEY_GRAVE_ACCENT: return "Backquote";

			// Arrow keys
		case GLFW_KEY_UP: return "ArrowUp";
		case GLFW_KEY_DOWN: return "ArrowDown";
		case GLFW_KEY_LEFT: return "ArrowLeft";
		case GLFW_KEY_RIGHT: return "ArrowRight";

			// Control keys
		case GLFW_KEY_ESCAPE: return "Escape";
		case GLFW_KEY_ENTER: return "Enter";
		case GLFW_KEY_TAB: return "Tab";
		case GLFW_KEY_BACKSPACE: return "Backspace";
		case GLFW_KEY_INSERT: return "Insert";
		case GLFW_KEY_DELETE: return "Delete";
		case GLFW_KEY_PAGE_UP: return "PageUp";
		case GLFW_KEY_PAGE_DOWN: return "PageDown";
		case GLFW_KEY_HOME: return "Home";
		case GLFW_KEY_END: return "End";
		case GLFW_KEY_CAPS_LOCK: return "CapsLock";
		case GLFW_KEY_SCROLL_LOCK: return "ScrollLock";
		case GLFW_KEY_NUM_LOCK: return "NumLock";
		case GLFW_KEY_PRINT_SCREEN: return "PrintScreen";
		case GLFW_KEY_PAUSE: return "Pause";

			// Numpad
		case GLFW_KEY_KP_0: return "Numpad0";
		case GLFW_KEY_KP_1: return "Numpad1";
		case GLFW_KEY_KP_2: return "Numpad2";
		case GLFW_KEY_KP_3: return "Numpad3";
		case GLFW_KEY_KP_4: return "Numpad4";
		case GLFW_KEY_KP_5: return "Numpad5";
		case GLFW_KEY_KP_6: return "Numpad6";
		case GLFW_KEY_KP_7: return "Numpad7";
		case GLFW_KEY_KP_8: return "Numpad8";
		case GLFW_KEY_KP_9: return "Numpad9";
		case GLFW_KEY_KP_DECIMAL: return "NumpadDecimal";
		case GLFW_KEY_KP_DIVIDE: return "NumpadDivide";
		case GLFW_KEY_KP_MULTIPLY: return "NumpadMultiply";
		case GLFW_KEY_KP_SUBTRACT: return "NumpadSubtract";
		case GLFW_KEY_KP_ADD: return "NumpadAdd";
		case GLFW_KEY_KP_ENTER: return "NumpadEnter";
		case GLFW_KEY_KP_EQUAL: return "NumpadEqual";

			// Modifiers (usually not used as main key, but included for completeness)
		case GLFW_KEY_LEFT_SHIFT: return "ShiftLeft";
		case GLFW_KEY_LEFT_CONTROL: return "ControlLeft";
		case GLFW_KEY_LEFT_ALT: return "AltLeft";
		case GLFW_KEY_LEFT_SUPER: return "MetaLeft";
		case GLFW_KEY_RIGHT_SHIFT: return "ShiftRight";
		case GLFW_KEY_RIGHT_CONTROL: return "ControlRight";
		case GLFW_KEY_RIGHT_ALT: return "AltRight";
		case GLFW_KEY_RIGHT_SUPER: return "MetaRight";

		default: return "Unknown";
		}
	}

	inline std::string KeyToJSKey(int key) {
		// Letters
		if (key >= GLFW_KEY_A && key <= GLFW_KEY_Z) {
			return "Key" + std::string(1, 'A' + (key - GLFW_KEY_A));
		}

		// Numbers
		if (key >= GLFW_KEY_0 && key <= GLFW_KEY_9) {
			return "Digit" + std::string(1, '0' + (key - GLFW_KEY_0));
		}

		// Function keys
		if (key >= GLFW_KEY_F1 && key <= GLFW_KEY_F25) {
			return "F" + std::to_string(key - GLFW_KEY_F1 + 1);
		}

		// Special keys
		switch (key) {
		case GLFW_KEY_SPACE: return "Space";
		case GLFW_KEY_APOSTROPHE: return "Quote";
		case GLFW_KEY_COMMA: return "Comma";
		case GLFW_KEY_MINUS: return "Minus";
		case GLFW_KEY_PERIOD: return "Period";
		case GLFW_KEY_SLASH: return "Slash";
		case GLFW_KEY_SEMICOLON: return "Semicolon";
		case GLFW_KEY_EQUAL: return "Equal";
		case GLFW_KEY_LEFT_BRACKET: return "BracketLeft";
		case GLFW_KEY_BACKSLASH: return "Backslash";
		case GLFW_KEY_RIGHT_BRACKET: return "BracketRight";
		case GLFW_KEY_GRAVE_ACCENT: return "Backquote";

			// Arrow keys
		case GLFW_KEY_UP: return "ArrowUp";
		case GLFW_KEY_DOWN: return "ArrowDown";
		case GLFW_KEY_LEFT: return "ArrowLeft";
		case GLFW_KEY_RIGHT: return "ArrowRight";

			// Control keys
		case GLFW_KEY_ESCAPE: return "Escape";
		case GLFW_KEY_ENTER: return "Enter";
		case GLFW_KEY_TAB: return "Tab";
		case GLFW_KEY_BACKSPACE: return "Backspace";
		case GLFW_KEY_INSERT: return "Insert";
		case GLFW_KEY_DELETE: return "Delete";
		case GLFW_KEY_PAGE_UP: return "PageUp";
		case GLFW_KEY_PAGE_DOWN: return "PageDown";
		case GLFW_KEY_HOME: return "Home";
		case GLFW_KEY_END: return "End";
		case GLFW_KEY_CAPS_LOCK: return "CapsLock";
		case GLFW_KEY_SCROLL_LOCK: return "ScrollLock";
		case GLFW_KEY_NUM_LOCK: return "NumLock";
		case GLFW_KEY_PRINT_SCREEN: return "PrintScreen";
		case GLFW_KEY_PAUSE: return "Pause";

			// Numpad
		case GLFW_KEY_KP_0: return "Numpad0";
		case GLFW_KEY_KP_1: return "Numpad1";
		case GLFW_KEY_KP_2: return "Numpad2";
		case GLFW_KEY_KP_3: return "Numpad3";
		case GLFW_KEY_KP_4: return "Numpad4";
		case GLFW_KEY_KP_5: return "Numpad5";
		case GLFW_KEY_KP_6: return "Numpad6";
		case GLFW_KEY_KP_7: return "Numpad7";
		case GLFW_KEY_KP_8: return "Numpad8";
		case GLFW_KEY_KP_9: return "Numpad9";
		case GLFW_KEY_KP_DECIMAL: return "NumpadDecimal";
		case GLFW_KEY_KP_DIVIDE: return "NumpadDivide";
		case GLFW_KEY_KP_MULTIPLY: return "NumpadMultiply";
		case GLFW_KEY_KP_SUBTRACT: return "NumpadSubtract";
		case GLFW_KEY_KP_ADD: return "NumpadAdd";
		case GLFW_KEY_KP_ENTER: return "NumpadEnter";
		case GLFW_KEY_KP_EQUAL: return "NumpadEqual";

			// Modifiers (usually not used as main key, but included for completeness)
		case GLFW_KEY_LEFT_SHIFT: return "ShiftLeft";
		case GLFW_KEY_LEFT_CONTROL: return "ControlLeft";
		case GLFW_KEY_LEFT_ALT: return "AltLeft";
		case GLFW_KEY_LEFT_SUPER: return "MetaLeft";
		case GLFW_KEY_RIGHT_SHIFT: return "ShiftRight";
		case GLFW_KEY_RIGHT_CONTROL: return "ControlRight";
		case GLFW_KEY_RIGHT_ALT: return "AltRight";
		case GLFW_KEY_RIGHT_SUPER: return "MetaRight";

		default: return "Unknown";
		}
	}

	enum class StringRepresentation
	{
		UI,
		Javascript,
	};

	inline std::string ToString(int key, int mods, StringRepresentation repr) {
		std::vector<std::string> parts;

		if ((mods & GLFW_KEY_LEFT_CONTROL) || (mods & GLFW_KEY_RIGHT_CONTROL))
			parts.push_back("Ctrl");

		if ((mods & GLFW_KEY_RIGHT_SHIFT) || (mods & GLFW_KEY_LEFT_SHIFT))
			parts.push_back("Shift");

		if ((mods & GLFW_KEY_RIGHT_ALT) || (mods & GLFW_KEY_LEFT_ALT))
			parts.push_back("Alt");

		if ((mods & GLFW_KEY_RIGHT_SUPER) || (mods & GLFW_KEY_LEFT_SUPER))
			parts.push_back("Meta");

		switch (repr)
		{
		case StringRepresentation::UI:
			parts.push_back(KeyToUIKey(key));
			break;
		case StringRepresentation::Javascript:
			parts.push_back(KeyToJSKey(key));
			break;
		}
		
		std::string result;
		for (size_t i = 0; i < parts.size(); ++i) {
			result += parts[i];
			if (i < parts.size() - 1) {
				result += "+";
			}
		}

		return result;
	}
}