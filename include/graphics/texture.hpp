#pragma once

#include "util/image.hpp"
#include "util/uix.hpp"


struct texture_color {
	ztu::u8 r, g, b, a;
};

using texture = ztu::image<texture_color>;
