#pragma once

#include <utility>

namespace ztu::for_each {

	template<typename... Types>
	inline constexpr bool type(auto &&f) {
		return (f.template operator()<Types>() || ...);
	}

	template<auto... Values>
	inline constexpr bool value(auto &&f) {
		return (f.template operator()<Values>() || ...);
	}

	template<typename... Args>
	inline constexpr bool argument(auto &&f, Args&&... args) {
		return (f(std::forward<Args>(args)) || ...);
	}

	template <auto Size>
	inline constexpr bool index(auto&& f) {
		return [&]<auto... Indices>(std::index_sequence<Indices...>) {
			return (f.template operator()<Indices>() || ...);
		}(std::make_index_sequence<Size>());
	}

	template<typename... Types>
	inline constexpr bool indexed_type(auto &&f) {
		return [&]<auto... Indices>(std::index_sequence<Indices...>) {
			return (f.template operator()<Indices, Types>() || ...);
		}(std::make_index_sequence<sizeof...(Types)>());
	}

	template<auto... Values>
	inline constexpr bool indexed_value(auto &&f) {
		return [&]<auto... Indices>(std::index_sequence<Indices...>) {
			return (f.template operator()<Indices, Values>() || ...);
		}(std::make_index_sequence<sizeof...(Values)>());
	}

	template<typename... Args>
	inline constexpr bool indexed_argument(auto &&f, Args&&... args) {
		return [&]<auto... Indices>(std::index_sequence<Indices...>) {
			return (f.template operator()<Indices>(std::forward<Args>(args)) || ...);
		}(std::make_index_sequence<sizeof...(Args)>());
	}
}
