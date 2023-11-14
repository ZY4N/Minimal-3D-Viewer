#pragma once

#include <cstdint>
#include <unistd.h>
#include <limits>
#include <bit>
#include <concepts>
#include <cmath>


namespace ztu {

namespace uix {

using u8 = std::uint8_t;
using u16 = std::uint16_t;
using u32 = std::uint32_t;
using u64 = std::uint64_t;
using usize = std::size_t;

using i8 = std::int8_t;
using i16 = std::int16_t;
using i32 = std::int32_t;
using i64 = std::int64_t;
using isize = ssize_t;

[[maybe_unused]] static constexpr auto u8_max = std::numeric_limits<u8>::max();
[[maybe_unused]] static constexpr auto u16_max = std::numeric_limits<u16>::max();
[[maybe_unused]] static constexpr auto u32_max = std::numeric_limits<u32>::max();
[[maybe_unused]] static constexpr auto u64_max = std::numeric_limits<u64>::max();
[[maybe_unused]] static constexpr auto usize_max = std::numeric_limits<usize>::max();

[[maybe_unused]] static constexpr auto i8_max = std::numeric_limits<i8>::max();
[[maybe_unused]] static constexpr auto i16_max = std::numeric_limits<i16>::max();
[[maybe_unused]] static constexpr auto i32_max = std::numeric_limits<i32>::max();
[[maybe_unused]] static constexpr auto i64_max = std::numeric_limits<i64>::max();
[[maybe_unused]] static constexpr auto isize_max = std::numeric_limits<isize>::max();

[[maybe_unused]] static constexpr auto u8_min = std::numeric_limits<u8>::min();
[[maybe_unused]] static constexpr auto u16_min = std::numeric_limits<u16>::min();
[[maybe_unused]] static constexpr auto u32_min = std::numeric_limits<u32>::min();
[[maybe_unused]] static constexpr auto u64_min = std::numeric_limits<u64>::min();
[[maybe_unused]] static constexpr auto usize_min = std::numeric_limits<usize>::min();

[[maybe_unused]] static constexpr auto i8_min = std::numeric_limits<i8>::min();
[[maybe_unused]] static constexpr auto i16_min = std::numeric_limits<i16>::min();
[[maybe_unused]] static constexpr auto i32_min = std::numeric_limits<i32>::min();
[[maybe_unused]] static constexpr auto i64_min = std::numeric_limits<i64>::min();
[[maybe_unused]] static constexpr auto isize_min = std::numeric_limits<isize>::min();

} // namespace ztu

using namespace uix;

namespace detail {
template<isize Index, typename... Ts>
struct pack_at {};

template<isize Index, typename T, typename... Ts> requires (Index > 0)
struct pack_at<Index, T, Ts...> {
	using type = pack_at<Index - 1, Ts...>::type;
};

template<typename T, typename... Ts>
struct pack_at<0, T, Ts...> {
	using type = T;
};
}


template<usize MinBytes>
using uint_t = detail::pack_at<std::bit_width(2 * MinBytes - 1) - 1, u8, u16, u32, u64>::type;

template<std::integral auto N> requires (N > 0)
using uint_holding = uint_t<(std::bit_width(static_cast<u64>(N)) + 7) / 8>;


template<usize MinBytes>
using int_t = detail::pack_at<std::bit_width(2 * MinBytes - 1) - 1, i8, i16, i32, i64>::type;

template<std::integral auto N> requires (N != 0)
using int_holding = int_t<
	std::bit_width(
		N < 0 ?
			std::max(static_cast<u64>(-(N + 1)), static_cast<u64>(1)) :
			static_cast<u64>(N)
	) / 8 + 1
>;

} // namespace ztu
