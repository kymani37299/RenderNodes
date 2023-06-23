#pragma once

#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_node_editor.h>
#include <imgui_internal.h>
#include <memory>
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

#define FORCE_CRASH *((unsigned int*)0) = 0xDEAD
#define STATIC_ARRAY_SIZE(X) (sizeof(X)/(sizeof(X[0])))

inline void GLClearError()
{
	while (glGetError());
}

inline bool GLCheckError()
{
	bool hasError = false;
	while (GLenum error = glGetError())
	{
		std::cout << "[OpenGL Error] " << error << std::endl;
		hasError = true;
	}
	return hasError;
}

#ifdef _DEBUG
#define ASSERT_M(X,msg) do { if(!(X)) { std::cout << msg << std::endl; __debugbreak(); } } while(0)
#define GL_CALL(X) GLClearError();X;ASSERT(!GLCheckError())
#else
#define ASSERT_M(X, msg) {}
#define GL_CALL(X) X
#endif // DEBUG

#define ASSERT(X) ASSERT_M(X, "Assert failed!")
#define NOT_IMPLEMENTED ASSERT_M(0, "NOT IMPLEMENTED")

namespace ImNode = ax::NodeEditor;

template<typename T>
using Ptr = std::unique_ptr<T>;

template<typename T>
using SharedPtr = std::shared_ptr<T>;

template<typename T> inline static constexpr uint32_t EnumToInt(T enumValue) { return static_cast<uint32_t>(enumValue); }
template<typename T> inline static constexpr T IntToEnum(uint32_t intValue) { return static_cast<T>(intValue); }

using Float2 = glm::vec2;
using Float3 = glm::vec3;
using Float4 = glm::vec4;

using Float4x4 = glm::mat4;
using Quaternion = glm::quat;