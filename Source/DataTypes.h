#pragma once

#include <memory>
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

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