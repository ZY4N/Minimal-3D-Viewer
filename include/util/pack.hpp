#pragma once

#include <utility>

namespace ztu {
	using std::size_t;

	template<typename... Ts>
	struct pack {};

	namespace packs {
		template<class T>
		struct of {};

		template<template<typename...> class T, typename... Ts>
		struct of<T<Ts...>> {
			using type = pack<Ts...>;
		};
	}

	template<class T>
	using pack_of = packs::of<T>::type;


	namespace packs {
		template<size_t Index, typename... Ts>
			requires (Index < sizeof...(Ts))
		struct at {};

		template<size_t Index, typename T, typename... Ts>
			requires (Index > 0)
		struct at<Index, T, Ts...> {
			using type = at<Index - 1, Ts...>::type;
		};

		template<typename T, typename... Ts>
		struct at<0, T, Ts...> {
			using type = T;
		};
	}

	template<size_t Index, typename... Ts>
	using at = packs::at<Index, Ts...>::type;


	namespace packs {
		template<typename T, typename... Ts>
		struct first {
			using type = T;
		};
	}

	template<typename... Ts>
	using first = packs::first<Ts...>::type;

	namespace packs {
		template<typename... Ts>
		struct last {};

		template<typename T, typename... Ts>
		struct last<T, Ts...> {
			using type = last<Ts...>::type;
		};

		template<typename T>
		struct last<T> {
			using type = T;
		};
	}

	template<typename... Ts>
	using last = packs::last<Ts...>::type;

	namespace packs {
		template<typename Type, typename... Ts>
		struct contains {
			static constexpr auto value = false;
		};

		template<typename Type, typename T, typename... Ts>
		struct contains<Type, T, Ts...> {
			static constexpr auto value = std::is_same_v<T, Type> or contains<Type, Ts...>::value;
		};
	};

	template<typename Type, typename... Ts>
	constexpr auto contains = packs::contains<Type, Ts...>::value;


	namespace packs {
		template<size_t index, typename Type, typename... Ts>
		struct index_of {};

		template<size_t Index, typename Type, typename T, typename... Ts>
		struct index_of<Index, Type, T, Ts...> {
			static constexpr auto index_of_rest = index_of<Index + 1, Type, Ts...>::value;
			static constexpr auto value = std::is_same_v<Type, T> ? Index : index_of_rest;
		};

		template<size_t Index, typename Type>
		struct index_of<Index, Type> {
			static constexpr size_t value = Index;
		};
	};

	template<typename Type, typename... Ts>
	static constexpr auto index_of = packs::index_of<0, Type, Ts...>::value;


	namespace packs {
		template<template<typename> class Pred, typename... Ts>
		struct find {};

		template<template<typename> class Pred, typename T, typename... Ts>
		struct find<Pred, T, Ts...> {
			struct type_wrapper { using type = T; };
			using type = std::conditional_t<Pred<T>::value,
				type_wrapper,
				find<Pred, Ts...>
			>::type;
		};
	};

	template<template<typename> class Pred, typename... Ts>
	using find = packs::find<Pred, Ts...>::type;


	namespace packs {
		template<size_t Index, template<size_t, typename> class Pred, typename... Ts>
		struct indexed_find {};

		template<size_t Index, template<size_t, typename> class Pred, typename T, typename... Ts>
		struct indexed_find<Index, Pred, T, Ts...> {
			struct type_wrapper { using type = T; };
			using type = std::conditional_t<Pred<Index, T>::value,
				type_wrapper,
				indexed_find<Index + 1, Pred, Ts...>
			>::type;
		};
	};

	template<template<size_t, typename> class Pred, typename... Ts>
	using indexed_find = packs::indexed_find<0, Pred, Ts...>::type;


	namespace packs {
		template<size_t index, typename Type, typename... Ts>
		struct index_of_last {};

		template<size_t Index, typename Type, typename U, typename... Ts>
		struct index_of_last<Index, Type, U, Ts...> {
			static constexpr auto index_of_rest = index_of_last<Index + 1, Type, Ts...>::value;
			static constexpr auto value = std::is_same_v<Type, U> and index_of_rest == -1 ? Index : index_of_rest;
		};

		template<size_t Index, typename Type>
		struct index_of_last<Index, Type> {
			static constexpr size_t value = -1;
		};
	};

	template<typename Type, typename... Ts>
	static constexpr auto index_of_last = packs::index_of_last<0, Type, Ts...>::value;


	namespace packs {
		template<template<typename> class Pred, typename... Ts>
		struct all_of {};

		template<template<typename> class Pred, typename T, typename... Ts>
		struct all_of<Pred, T, Ts...> {
			static constexpr auto allOf_rest = all_of<Pred, Ts...>::value;
			static constexpr auto value = Pred<T>::value and allOf_rest;
		};

		template<template<typename> class Pred>
		struct all_of<Pred> {
			static constexpr size_t value = -1;
		};
	};

	template<template<typename> class Pred, typename... Ts>
	static constexpr auto all_of = packs::all_of<Pred, Ts...>::value;


	namespace packs {
		template<template<typename> class Pred, typename... Ts>
		struct any_of {};

		template<template<typename> class Pred, typename T, typename... Ts>
		struct any_of<Pred, T, Ts...> {
			static constexpr auto anyOf_rest = any_of<Pred, Ts...>::value;
			static constexpr auto value = Pred<T>::value or anyOf_rest;
		};

		template<template<typename> class Pred>
		struct any_of<Pred> {
			static constexpr size_t value = -1;
		};
	};

	template<template<typename> class Pred, typename... Ts>
	static constexpr auto any_of = packs::any_of<Pred, Ts...>::value;


	namespace packs {
		template<class T, template<typename,typename> class Cmp, typename... Us>
		struct matches_all {};

		template<template<typename...> class T, typename... Ts, template<typename,typename> class Cmp, typename... Us>
		struct matches_all<T<Ts...>, Cmp, Us...> {
			static constexpr auto value = (
				Cmp<Ts, Us>::value and ...
			);
		};
	}

	template<class T, template<typename,typename> class Cmp, typename... Us>
	static constexpr auto matches_all = packs::matches_all<T, Cmp, Us...>::value;


	namespace packs {
		template<class T, template<typename,typename> class Cmp, typename... Us>
		struct matches_any {};

		template<template<typename...> class T, typename... Ts, template<typename,typename> class Cmp, typename... Us>
		struct matches_any<T<Ts...>, Cmp, Us...> {
			static constexpr auto value = (
				Cmp<Ts, Us>::value or ...
			);
		};
	}

	template<class T, template<typename,typename> class Cmp, typename... Us>
	static constexpr auto matches_any = packs::matches_any<T, Cmp, Us...>::value;


	namespace packs {
		template<template<typename, typename> class F, typename Init, typename... Ts>
		struct accumulate_reverse {};

		template<template<typename, typename> class F, typename Init, typename... Ts, typename U>
		struct accumulate_reverse<F, Init, U, Ts...> {
			using folded_rest = accumulate_reverse<F, Init, Ts...>::type;
			using type = F<folded_rest, U>::type;
		};

		template<template<typename, typename> class F, typename Init>
		struct accumulate_reverse<F, Init> {
			using type = Init;
		};
	};

	template<template<typename, typename> class F, typename Init, typename... Ts>
	using accumulate_reverse = packs::accumulate_reverse<F, Init, Ts...>::type;

	namespace packs {
		template<template<typename, typename> class F, typename Init, class T>
		struct accumulate_reverse_pack {};

		template<template<typename, typename> class F, typename Init, template<typename...> class T, typename... Ts>
		struct accumulate_reverse_pack<F, Init, T<Ts...>> {
			using type = accumulate_reverse<F, Init, Ts...>::type;
		};
	}

	template<template<typename, typename> class F, typename Init, class T>
	using accumulate_reverse_pack = packs::accumulate_reverse_pack<F, Init, T>::type;


	namespace packs {
		template<typename... Us>
		struct reverse {};

		template<typename U, typename... Us>
		struct reverse<U, Us...> {
			using type = reverse<Us...>::type::template append<U>;
		};

		template<>
		struct reverse<> {
			using type = pack<>;
		};
	}

	template<typename... Ts>
	using reverse = packs::reverse<Ts...>::type;


	namespace packs {
		template<class T, typename... Ts>
		struct append {};

		template<template<typename...> class T, typename... Ts, typename... Us>
		struct append<T<Ts...>, Us...> {
			using type = pack<Ts..., Us...>;
		};
	}

	template<class T, typename... Ts>
	using append = packs::append<T, Ts...>::type;


	namespace packs {
		template<typename T, typename... Ts>
		struct prepend {};

		template<template<typename...> class T, typename... Ts, typename... Us>
		struct prepend<T<Ts...>, Us...> {
			using type = pack<Us..., Ts...>;
		};
	}

	template<typename T, typename... Ts>
	using prepend = packs::prepend<T, Ts...>::type;


	namespace packs {
		template<class T, class U>
		struct append_pack {};

		template<class T, template<typename...> class U, typename... Us>
		struct append_pack<T, U<Us...>> {
			using type = append<T, Us...>::type;
		};
	}

	template<class T, class U>
	using append_pack = packs::append_pack<T, U>::type;


	namespace packs {
		template<class T, class U>
		struct prepend_pack {};

		template<class T, template<typename...> class U, typename... Us>
		struct prepend_pack<T, U<Us...>> {
			using type = append<T, Us...>::type;
		};
	}

	template<class T, class U>
	using prepend_pack = packs::prepend_pack<T, U>::type;


	namespace packs {
		template<template<typename> class Pred, typename... Ts>
		struct filter {};

		template<template<typename> class Pred, typename T, typename... Ts>
		struct filter<Pred, T, Ts...> {
			using filtered_rest = filter<Pred, Ts...>::type;
			using type = std::conditional_t<
				Pred<T>::value,
				typename prepend<filtered_rest, T>::type,
				filtered_rest
			>;
		};

		template<template<typename> class Pred>
		struct filter<Pred> {
			using type = pack<>;
		};
	};

	template<template<typename> class Pred, typename... Ts>
	using filter = packs::filter<Pred, Ts...>::type;


	namespace packs {
		template<size_t N, typename... Ts>
			requires (N <= sizeof...(Ts))
		struct erase_first_n {};

		template<size_t N, typename T, typename... Ts>
			requires (N > 0)
		struct erase_first_n<N, T, Ts...> {
			using type = erase_first_n<N - 1, Ts...>::type;
		};

		template<typename... Ts>
		struct erase_first_n<0, Ts...> {
			using type = pack<Ts...>;
		};
	};

	template<size_t N, typename... Ts>
	using erase_first_n = packs::erase_first_n<N, Ts...>::type;

	namespace packs {
		template<size_t N, size_t NumTs, typename... Ts>
			requires (N <= sizeof...(Ts))
		struct erase_last_n_impl {};

		template<size_t N, size_t NumTs, typename T, typename... Ts>
			requires (N < NumTs)
		struct erase_last_n_impl<N, NumTs, T, Ts...> {
			using erased_rest = erase_last_n_impl<N, sizeof...(Ts), Ts...>::type;
			using type = prepend<erased_rest, T>::type;
		};

		template<size_t N, typename... Ts>
		struct erase_last_n_impl<N, N, Ts...> {
			using type = ztu::pack<>;
		};

		template<size_t N, typename... Ts>
		struct erase_last_n {
			using type = erase_last_n_impl<N, sizeof...(Ts), Ts...>::type;
		};
	};

	template<size_t N, typename... Ts>
	using erase_last_n = packs::erase_last_n<N, Ts...>::type;


	namespace packs {
		template<size_t First, size_t Last, typename... Ts>
			requires (First < sizeof...(Ts) and Last < sizeof...(Ts) and First <= Last)
		struct erase {};

		template<size_t First, size_t Last, typename T, typename... Ts>
			requires (First > 0)
		struct erase<First, Last, T, Ts...> {
			using erased_rest = erase<First - 1, Last - 1, Ts...>::type;
			using type = prepend<erased_rest, T>::type;
		};

		template<size_t Last, typename... Ts>
		struct erase<0, Last, Ts...> {
			using type = erase_first_n<Last + 1, Ts...>::type;
		};
	};

	template<size_t first, size_t last, typename... Ts>
	using erase = packs::erase<first, last, Ts...>::type;


	namespace packs {
		template<class T, size_t Index, typename... Us>
			requires (Index < T::size)
		struct insert {};

		template<template<typename...> class PT, typename T, typename... Ts, size_t Index, typename... Us>
			requires (Index > 0)
		struct insert<PT<T, Ts...>, Index, Us...> {
			using inserted_rest = insert<PT<Ts...>, Index - 1, Us...>::type;
			using type = prepend<inserted_rest, T>::type;
		};

		template<class T, typename... Us>
		struct insert<T, 0, Us...> {
			using type = prepend<T, Us...>::type;
		};
	};

	template<class T, size_t Index, typename... Us>
	using insert = packs::insert<T, Index, Us...>::type;

	namespace packs {
		template<class T, size_t First, size_t Last>
			requires (First < T::size and Last < T::size and First <= Last)
		struct extract {};

		template<template<typename...> class T, typename... LsTsRs, size_t First, size_t Last>
			requires (First > 0)
		struct extract<T<LsTsRs...>, First, Last> {
			using TsRs = erase_first_n<First, LsTsRs...>::type;
			using type = extract<TsRs, 0, Last - First>::type;
		};

		template<template<typename...> class T, typename... TsRs, size_t Last>
		struct extract<T<TsRs...>, 0, Last> {
			using type = erase_last_n<sizeof...(TsRs) - 1 - Last, TsRs...>::type;
		};
	};

	template<size_t First, size_t Last, typename... Ts>
	using extract = packs::extract<pack<Ts...>, First, Last>::type;


	namespace packs {
		template<template<typename> class F, typename... Ts>
		struct transform {};

		template<template<typename> class F, typename T, typename... Ts>
		struct transform<F, T, Ts...> {
			using type = transform<F, Ts...>::type::template prepend<typename F<T>::type>;
		};

		template<template<typename> class F>
		struct transform<F> {
			using type = pack<>;
		};
	};

	template<template<typename> class F, typename... Ts>
	using transform = packs::transform<F, Ts...>::type;

	namespace packs {
		template<template<typename, typename> class Less, typename... Us>
		struct sort_iteration{};

		template<template<typename, typename> class Less, typename U, typename V, typename... Ts>
		struct sort_iteration<Less, U, V, Ts...>{
			static constexpr auto sorted = not Less<V, U>::value;
			using First = std::conditional_t<sorted, U, V>;
			using Second = std::conditional_t<sorted, V, U>;
			using sorted_rest = sort_iteration<Less, Second, Ts...>;
			static constexpr auto done = sorted and sorted_rest::done;
			using type = prepend<typename sorted_rest::type, First>::type;
		};

		template<template<typename, typename> class Less, typename U>
		struct sort_iteration<Less, U>{
			static constexpr auto done = true;
			using type = pack<U>;
		};

		template<template<typename, typename> class Less, bool Sorted, class T>
		struct sort_pack {
			using type = T;
		};

		template<template<typename, typename> class Less, template<typename...> class T, typename... Ts>
		struct sort_pack<Less, false, T<Ts...>> {
			using iteration = sort_iteration<Less, Ts...>;
			using type = sort_pack<Less, iteration::done, typename iteration::type>::type;
		};

		template<template<typename, typename> class Less, bool Sorted, typename... Ts>
		struct sort {
			using type = pack<Ts...>;
		};

		template<template<typename, typename> class Less, typename... Ts>
		struct sort<Less, false, Ts...> {
			using iteration = sort_iteration<Less, Ts...>;
			using type = sort_pack<Less, iteration::done, typename iteration::type>::type;
		};
	};

	template<template<typename, typename> class Less, typename... Ts>
	using sort = packs::sort<Less, false, Ts...>::type;

	template<template<typename, typename> class Less, class T>
	using sort_pack = packs::sort_pack<Less, false, T>::type;


	namespace packs {
		template<class F, typename... Ts>
		inline constexpr bool for_each(F&& f) {
			return (f.template operator()<Ts>() || ...);
		}

		template<class F, typename... Ts>
		inline constexpr bool indexed_for_each(F&& f) {
			return [&]<auto... Is>(std::index_sequence<Is...>) {
				return (f.template operator()<Is, Ts>() || ...);
			}(std::index_sequence_for<Ts...>{});
		}

		template<class F, typename... Ts>
		inline constexpr size_t index_of_f(F&& f) {
			return [&]<auto... Is>(std::index_sequence<Is...>) {
				size_t index = 0;
				const auto foundIt = ((index = Is, f.template operator()<Ts>()) || ...);
				return index + not foundIt;
			}(std::index_sequence_for<Ts...>{});
		}

		template<bool FLipIndices, class F, typename... Ts>
		inline constexpr size_t indexed_index_of_f(F&& f) {
			return [&]<auto... Is>(std::index_sequence<Is...>) {
				size_t index = 0;
				const auto foundIt = (
					(
						index = Is,
						f.template operator()<
							FLipIndices ? sizeof...(Ts) - 1 - Is : Is, Ts
						>()
					) || ...
				);
				return index + not foundIt;
			}(std::index_sequence_for<Ts...>{});
		}

		template<class F, typename... Ts>
		inline constexpr decltype(auto) apply_f(F &&f) {
			return f.template operator()<Ts...>();
		}

		template<class F, typename... Ts>
		inline constexpr bool all_of_f(F &&f) {
			return (f.template operator()<Ts>() and ...);
		}

		template<class F, typename... Ts>
		inline constexpr bool indexed_all_of_f(F &&f) {
			return [&]<auto... Is>(std::index_sequence<Is...>) {
				return (f.template operator()<Is, Ts>() and ...);
			}(std::index_sequence_for<Ts...>{});
		}

		template<class F, typename... Ts>
		inline constexpr bool any_of_f(F &&f) {
			return (f.template operator()<Ts>() or ...);
		}

		template<class F, typename... Ts>
		inline constexpr bool indexed_any_of_f(F &&f) {
			return [&]<auto... Is>(std::index_sequence<Is...>) {
				return (f.template operator()<Is, Ts>() or ...);
			}(std::index_sequence_for<Ts...>{});
		}

		template<typename T, class F, typename... Ts>
		inline constexpr T accumulate_f(F &&f, T value) {
			((value = f.template operator()<Ts>(value)), ...);
			return value;
		}

		template<typename T, class F, typename... Ts>
		inline constexpr T indexed_accumulate_f(F &&f, T value) {
			[&]<auto... Is>(std::index_sequence<Is...>) {
				((value = f.template operator()<Is, Ts>(value)), ...);
			}(std::index_sequence_for<Ts...>{});
			return value;
		}
	}

	template<>
	struct pack<> {
		using size_t = std::size_t;

		using self = pack<>;

		using index_sequence = std::index_sequence_for<>;

		static constexpr auto size = 0;

		static constexpr auto empty = true;

		template<typename Type>
		static constexpr auto index_of = size;

		template<typename Type>
		static constexpr auto index_of_last = size;

		template<template<typename> class Pred>
		static constexpr auto all_of = true;

		template<template<typename> class Pred>
		static constexpr auto any_of = false;

		template<template<typename,typename> class Cmp, typename... Us>
		static constexpr auto matches_all = true;

		template<template<typename,typename> class Cmp, typename... Us>
		static constexpr auto matches_any = false;

		template<typename... Us>
		using append = pack<Us...>;

		template<typename... Us>
		using prepend = pack<Us...>;

		template<class U>
		using append_pack = U;

		template<class U>
		using prepend_pack = U;

		template<size_t N = 1>
		using erase_first = ztu::erase_first_n<N>;

		template<size_t Index, typename... Us>
		using insert = ztu::insert<self, Index>;

		using reverse = self;

		template<template<typename> class Pred>
		using filter = self;

		template<template<typename> class F>
		using transform = self;

		template<template<typename...> class F>
		using apply = F<>;

		template<template<typename, typename> class F, typename Init>
		using accumulate_reverse = Init;

		template<template<typename, typename> class F, typename Init>
		using accumulate = Init;

		template<class F>
		static inline constexpr bool for_each(F&& f) {
			return false;
		}

		template<class F>
		static inline constexpr bool indexed_for_each(F&& f) {
			return false;
		}

		template<class F>
		static inline constexpr size_t index_of_f(F&& f) {
			return size;
		}

		template<bool FlipIndices = false, class F>
		static inline constexpr size_t indexed_index_of_f(F&& f) {
			return size;
		}

		template<class F>
		static inline constexpr size_t index_of_last_f(F&& f) {
			return size;
		}

		template<class F>
		static inline constexpr size_t indexed_index_of_last_f(F&& f) {
			return size;
		}

		template<class F>
		static inline constexpr decltype(auto) apply_f(F &&f) {
			return packs::apply_f<F>(std::forward<F>(f));
		}

		template<class F>
		static inline constexpr bool all_of_f(F &&f) {
			return true;
		}

		template<class F>
		static inline constexpr bool indexed_all_of_f(F &&f) {
			return true;
		}

		template<class F>
		static inline constexpr bool any_of_f(F &&f) {
			return false;
		}

		template<class F>
		static inline constexpr bool indexed_any_of_f(F &&f) {
			return false;
		}

		template<typename T, class F>
		static inline constexpr T accumulate_f(F &&f, T value) {
			return value;
		}

		template<typename T, class F>
		static inline constexpr T indexed_accumulate_f(F &&f, T value) {
			return value;
		}
	};

	template<typename T, typename... Ts>
	struct pack<T, Ts...> {
		using size_t = std::size_t;

		using self = pack<T, Ts...>;

		using index_sequence = std::index_sequence_for<T, Ts...>;

		using first = T;
		using last = ztu::last<T, Ts...>;

		template<size_t index>
		using at = ztu::at<index, T, Ts...>;

		template<size_t First, size_t Last = First>
		using erase = ztu::erase<First, Last, T, Ts...>;

		template<size_t First, size_t Last = First>
		using extract = ztu::extract<First, Last, T, Ts...>;

		template<template<typename, typename> class Less>
		using sort = ztu::sort<Less, T, Ts...>;

		static constexpr auto size = sizeof...(Ts);

		static constexpr auto empty = size == 0;

		template<template<typename> class Pred>
		using find = ztu::find<Pred, T, Ts...>;

		template<template<size_t, typename> class Pred>
		using indexed_find = ztu::indexed_find<Pred, T, Ts...>;

		template<typename Type>
		static constexpr auto index_of = ztu::index_of<Type, T, Ts...>;

		template<typename Type>
		static constexpr auto index_of_last = ztu::index_of_last<Type, T, Ts...>;

		template<template<typename> class Pred>
		static constexpr auto all_of = ztu::all_of<Pred, T, Ts...>;

		template<template<typename> class Pred>
		static constexpr auto any_of = ztu::any_of<Pred, T, Ts...>;

		template<template<typename,typename> class Cmp, typename... Us>
		static constexpr auto matches_all = ztu::matches_all<self, Cmp, Us...>;

		template<template<typename,typename> class Cmp, typename... Us>
		static constexpr auto matches_any = ztu::matches_any<self, Cmp, Us...>;

		template<typename... Us>
		using append = ztu::append<self, Us...>;

		template<typename... Us>
		using prepend = ztu::prepend<self, Us...>;

		template<class U>
		using append_pack = ztu::append_pack<self, U>;

		template<class U>
		using prepend_pack = ztu::prepend_pack<self, U>;

		template<size_t N = 1>
		using erase_first = ztu::erase_first_n<N, T, Ts...>;

		template<size_t Index, typename... Us>
		using insert = ztu::insert<self, Index, Us...>;

		using reverse = ztu::reverse<T, Ts...>;

		template<template<typename> class Pred>
		using filter = ztu::filter<Pred, T, Ts...>;

		template<template<typename> class F>
		using transform = ztu::transform<F, T, Ts...>;

		template<template<typename...> class F>
		using apply = F<T, Ts...>;

		template<template<typename, typename> class F, typename Init>
		using accumulate_reverse = ztu::accumulate_reverse<F, Init, T, Ts...>;

		template<template<typename, typename> class F, typename Init>
		using accumulate = ztu::accumulate_reverse_pack<F, Init, ztu::reverse<T, Ts...>>;

		template<class F>
		static inline constexpr bool for_each(F&& f) {
			return packs::for_each<F, T, Ts...>(std::forward<F>(f));
		}

		template<class F>
		static inline constexpr bool indexed_for_each(F&& f) {
			return packs::indexed_for_each<F, T, Ts...>(std::forward<F>(f));
		}

		template<class F>
		static inline constexpr size_t index_of_f(F&& f) {
			return packs::index_of_f<F, T, Ts...>(std::forward<F>(f));
		}

		template<bool FlipIndices = false, class F>
		static inline constexpr size_t indexed_index_of_f(F&& f) {
			return packs::indexed_index_of_f<FlipIndices, F, T, Ts...>(std::forward<F>(f));
		}

		template<class F>
		static inline constexpr size_t index_of_last_f(F&& f) {
			const auto reverseIndex = reverse::template index_of<F>(std::forward<F>(f));
			return size - reverseIndex;
		}

		template<class F>
		static inline constexpr size_t indexed_index_of_last_f(F&& f) {
			const auto reverseIndex = reverse::template indexed_index_of_f<true, F>(std::forward<F>(f));
			return size - reverseIndex;
		}

		template<class F>
		static inline constexpr decltype(auto) apply_f(F &&f) {
			return packs::apply_f<F, T, Ts...>(std::forward<F>(f));
		}

		template<class F>
		static inline constexpr bool all_of_f(F &&f) {
			return packs::all_of_f<F, T, Ts...>(std::forward<F>(f));
		}

		template<class F>
		static inline constexpr bool indexed_all_of_f(F &&f) {
			return packs::indexed_all_of_f<F, T, Ts...>(std::forward<F>(f));
		}

		template<class F>
		static inline constexpr bool any_of_f(F &&f) {
			return packs::any_of_f<F, T, Ts...>(std::forward<F>(f));
		}

		template<class F>
		static inline constexpr bool indexed_any_of_f(F &&f) {
			return packs::indexed_any_of_f<F, T, Ts...>(std::forward<F>(f));
		}

		template<typename U, class F>
		static inline constexpr U accumulate_f(F &&f, U &&value) {
			return packs::accumulate_f<U, F, T, Ts...>(std::forward<F>(f), std::forward<U>(value));
		}

		template<typename U, class F>
		static inline constexpr U indexed_accumulate_f(F &&f, T value) {
			return packs::indexed_accumulate_f<U, F, T, Ts...>(std::forward<F>(f), std::forward<U>(value));
		}
	};
}
