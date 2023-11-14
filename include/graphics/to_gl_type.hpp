#pragma once

#include <utility>
#include <SFML/OpenGL.hpp>
#include "util/uix.hpp"


template<typename T>
consteval GLenum to_gl_type() {
	if constexpr (std::same_as<T, ztu::i8>) {
		return GL_BYTE;
	} else if constexpr (std::same_as<T, ztu::u8>) {
		return GL_UNSIGNED_BYTE;
	} else if constexpr (std::same_as<T, ztu::i16>) {
		return GL_SHORT;
	} else if constexpr (std::same_as<T, ztu::u16>) {
		return GL_UNSIGNED_SHORT;
	} else if constexpr (std::same_as<T, ztu::i32>) {
		return GL_INT;
	} else if constexpr (std::same_as<T, ztu::u32>) {
		return GL_UNSIGNED_INT;
	} else if constexpr (std::same_as<T, float>) {
		return GL_FLOAT;
	} else if constexpr (std::same_as<T, double>) {
		return GL_DOUBLE;
	} else {
		T::___unknown_type;
	}
}
