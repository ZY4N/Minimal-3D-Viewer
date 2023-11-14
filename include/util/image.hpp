#pragma once


#include <cstdint>
#include <memory>
#include <string>
#include <stdexcept>
#include <iostream>
#include <algorithm>


#if defined(__GNUC__) || defined(__GNUG__)
#pragma GCC diagnostic push
#pragma GCC system_header
#elif defined(_MSC_VER)
#pragma warning(push, 0)
#endif

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_STATIC
#include "stb_image.h"


#define STBI_MSC_SECURE_CRT
#define STB_IMAGE_WRITE_STATIC
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"


#if defined(__GNUC__) || defined(__GNUG__)
#pragma GCC diagnostic pop
#elif defined(_MSC_VER)
#pragma warning(pop)
#endif

namespace ztu {

template<typename C>
class image {
public:
	using this_type = image<C>;
	using value_type = C;
	using size_type = std::make_signed_t<std::size_t>;
	using difference_type = size_type;
	using reference = std::add_lvalue_reference_t<value_type>;
	using const_reference = std::add_const_t<reference>;
	using pointer = std::add_pointer_t<value_type>;
	using const_pointer = std::add_const_t<pointer>;
	using iterator = pointer;
	using const_iterator = const_pointer;

public:
	[[nodiscard]] inline static std::error_code load(const std::string& filename, image& dst, bool flip = false);

	[[nodiscard]] inline static image create(size_type width, size_type height, const value_type& color);

public:
	image() = default;

	image(std::unique_ptr<value_type>&& data, size_type width, size_type height);

	image(const image&);

	image(image&&) noexcept;

	[[nodiscard]] inline this_type& operator=(const image&);

	[[nodiscard]] inline this_type& operator=(image&&) noexcept;

public:
	[[nodiscard]] inline value_type operator()(double x, double y) const;

	[[nodiscard]] inline const_reference operator()(size_type x, size_type y) const;

	[[nodiscard]] inline reference operator()(size_type x, size_type y);

	[[nodiscard]] inline const_iterator operator[](size_type y) const;

	[[nodiscard]] inline iterator operator[](size_type y);

	[[nodiscard]] inline int save(const std::string& filename) const;

	[[nodiscard]] inline bool contains(size_type x, size_type y) const;

	[[nodiscard]] inline size_type width() const;

	[[nodiscard]] inline size_type height() const;

	[[nodiscard]] inline std::pair<size_type, size_type> size() const;

	[[nodiscard]] inline size_type num_pixels() const;

	[[nodiscard]] inline const_iterator begin() const;

	[[nodiscard]] inline iterator begin();

	[[nodiscard]] inline const_iterator end() const;

	[[nodiscard]] inline iterator end();

	[[nodiscard]] inline const_pointer data() const;

	[[nodiscard]] inline pointer data();

private:
	std::unique_ptr<value_type[]> m_data{ nullptr };
	size_type m_width{ 0 }, m_height{ 0 };
};

template<typename C>
image<C>::image(std::unique_ptr<value_type>&& data, const size_type width, const size_type height) :
	m_data{ std::move(data) }, m_width{ width }, m_height{ height } {
};

template<typename C>
image<C>::image(const image& other) :
	m_data{ new C[other.m_width * other.m_height] },
	m_width{ other.m_width }, m_height{ other.m_height } {
	std::copy_n(other.m_data, other.m_width * other.m_height, this->m_data);
}

template<typename C>
image<C>::image(image<C>&& other) noexcept :
	m_data{ std::move(other.m_data) },
	m_width{ other.m_width }, m_height{ other.m_height } {
	other.m_width = 0;
	other.m_height = 0;
}

template<typename C>
image<C>& image<C>::operator=(const image<C>& other) {
	if (this != &other) {

		const auto m_num_pixels = m_width * m_height;
		const auto o_num_pixels = other.m_width * other.m_height;

		if (o_num_pixels > m_num_pixels) {
			this->~image();
			this->m_data = new C[o_num_pixels];
		}

		std::copy_n(other.m_data, o_num_pixels, this->m_data);

		this->m_width = other.m_width;
		this->m_height = other.m_height;
	}
	return *this;
}

template<typename C>
image<C>& image<C>::operator=(image&& other) noexcept {
	if (this != &other) {
		this->~image();

		this->m_width = other.m_width;
		this->m_height = other.m_height;
		this->m_data = std::move(other.m_data);

		other.m_width = 0;
		other.m_height = 0;
	}
	return *this;
}


template<typename C>
std::error_code image<C>::load(const std::string& filename, image<C>& dst, bool flip) {
	int width, height, channels;

	stbi_set_flip_vertically_on_load(flip);
	auto data = reinterpret_cast<pointer>(stbi_load(filename.c_str(), &width, &height, &channels, sizeof(C)));

	if (data == nullptr) {
		return std::make_error_code(std::errc::no_such_file_or_directory);
	}

	dst.m_width = static_cast<size_type>(width);
	dst.m_height = static_cast<size_type>(height);
	dst.m_data.reset(data);

	return {};
}

template<typename C>
image<C> image<C>::create(const size_type width, const size_type height, const C& color) {

	const auto num_pixels = width * height;
	C* data = new C[num_pixels];

	std::fill_n(data, num_pixels, color);

	return image(data, width, height);
}

template<typename C>
image<C>::size_type image<C>::width() const {
	return m_width;
}

template<typename C>
image<C>::size_type image<C>::height() const {
	return m_width;
}

template<typename C>
std::pair<typename image<C>::size_type, typename image<C>::size_type> image<C>::size() const {
	return { m_width, m_height };
}

template<typename C>
image<C>::size_type image<C>::num_pixels() const {
	return m_width * m_height;
}

template<typename C>
int image<C>::save(const std::string& filename) const {

	std::string ext = filename.substr(filename.rfind('.') + 1, filename.length());
	std::transform(
		ext.begin(), ext.end(), ext.begin(), [](unsigned char c) {
			return std::tolower(c);
		}
	);

	int status = -1;

	if (ext == "png") {
		status = stbi_write_png(filename.c_str(), m_width, m_height, sizeof(C), m_data, m_width * sizeof(C));
	} else if (ext == "bmp") {
		status = stbi_write_bmp(filename.c_str(), m_width, m_height, sizeof(C), m_data);
	} else if (ext == "tga") {
		status = stbi_write_tga(filename.c_str(), m_width, m_height, sizeof(C), m_data);
	} else if (ext == "jpg" || ext == "jpeg") {
		status = stbi_write_jpg(filename.c_str(), m_width, m_height, sizeof(C), m_data, m_width * sizeof(C));
	}

	return status;
}

template<typename C>
bool image<C>::contains(size_type x, size_type y) const {
	return 0 <= x and x < m_width and 0 <= y and y < m_height;
}

template<typename C>
image<C>::const_reference image<C>::operator()(size_type x, size_type y) const {
	const auto clamped_x = std::clamp(x, static_cast<size_type>(0), m_width - 1);
	const auto clamped_y = std::clamp(y, static_cast<size_type>(0), m_height - 1);
	return m_data[clamped_x + clamped_y * m_width];
}

template<typename C>
image<C>::reference image<C>::operator()(size_type x, size_type y) {
	const auto clamped_x = std::clamp(x, static_cast<size_type>(0), m_width - 1);
	const auto clamped_y = std::clamp(y, static_cast<size_type>(0), m_height - 1);
	return m_data[clamped_x + clamped_y * m_width];
}

template<typename C>
image<C>::value_type image<C>::operator()(double x, double y) const {
	auto min_x = static_cast<size_type>(std::floor(x));
	auto min_y = static_cast<size_type>(std::floor(y));
	const auto px00 = (*this)(min_x, min_y), px10 = (*this)(min_x + 1, min_y);
	const auto px01 = (*this)(min_x, min_y + 1), px11 = (*this)(min_x + 1, min_y + 1);
	const auto a_x = x - static_cast<double>(min_x), a_y = y - static_cast<double>(min_y);
	return std::lerp(
		std::lerp(px00, px10, a_x),
		std::lerp(px01, px11, a_x),
		a_y
	);
}

template<typename C>
image<C>::const_iterator image<C>::operator[](size_type y) const {
	return &m_data[y * m_width];
}

template<typename C>
image<C>::iterator image<C>::operator[](size_type y) {
	return &m_data[y * m_width];
}


template<typename C>
image<C>::const_iterator image<C>::begin() const {
	return m_data.get();
}

template<typename C>
image<C>::iterator image<C>::begin() {
	return m_data.get();
}

template<typename C>
image<C>::const_iterator image<C>::end() const {
	return m_data.get() + m_width * m_height;
}

template<typename C>
image<C>::iterator image<C>::end() {
	return m_data.get() + m_width * m_height;
}

template<typename C>
image<C>::const_pointer image<C>::data() const {
	return m_data.get();
}

template<typename C>
image<C>::pointer image<C>::data() {
	return m_data.get();
}

} // namespace ztu
