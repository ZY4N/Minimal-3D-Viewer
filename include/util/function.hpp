#pragma once

#include <tuple>
#include <concepts>

namespace ztu {

	template<typename F, typename R, typename... Args>
	concept callable = std::same_as<std::invoke_result_t<F, Args...>, R>;

	template<typename T>
	concept runnable = callable<T, void>;

	template<typename T, typename R>
	concept supplier = callable<T, R>;

	template<typename T, typename... Args>
	concept consumer = callable<T, void, Args...>;

	template<typename T, typename... Args>
	concept predicate = callable<T, bool, Args...>;


	template<typename>
	struct function_meta;

	template<typename R, typename... Args>
	struct function_meta<R(*)(Args...)> {
		using ret_t = R;
		using args_t = std::tuple<Args...>;
		static constexpr bool is_const = false;
	};

	template<class C, typename R, typename... Args>
	struct function_meta<R(C::*)(Args...)> {
		using class_t = C;
		using ret_t = R;
		using args_t = std::tuple<Args...>;
		static constexpr bool is_const = false;
	};

	template<class C, typename R, typename... Args>
	struct function_meta<R(C::*)(Args...) const> {
		using class_t = C;
		using ret_t = R;
		using args_t = std::tuple<Args...>;
		static constexpr bool is_const = true;
	};

	namespace function {
		template<typename F>
		using class_t = typename function_meta<F>::class_t;

		template<typename F>
		using ret_t = typename function_meta<F>::ret_t;

		template<typename F>
		using args_t = typename function_meta<F>::args_t;

		template<typename F>
		constexpr bool is_const_v = function_meta<F>::is_const;
	}
}
