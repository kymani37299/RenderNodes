#pragma once

#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_node_editor.h>
#include <memory>

#define FORCE_CRASH *((unsigned int*)0) = 0xDEAD
#define STATIC_ARRAY_SIZE(X) (sizeof(X)/(sizeof(X[0])))

#ifdef _DEBUG
#define ASSERT_M(X,msg) do { if(!(X)) { std::cout << msg << std::endl; __debugbreak(); } } while(0)
#else
#define ASSERT_M(X, msg)
#endif // DEBUG

#define ASSERT(X) ASSERT_M(X, "Assert failed!")
#define NOT_IMPLEMENTED ASSERT_M(0, "NOT IMPLEMENTED")

namespace ImNode = ax::NodeEditor;

template<typename T>
using Ptr = std::unique_ptr<T>;

using UniqueID = uintptr_t;
using NodeID = UniqueID;
using PinID = UniqueID;
using LinkID = UniqueID;

template<typename T> inline static constexpr uint32_t EnumToInt(T enumValue) { return static_cast<uint32_t>(enumValue); }
template<typename T> inline static constexpr T IntToEnum(uint32_t intValue) { return static_cast<T>(intValue); }