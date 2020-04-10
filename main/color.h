#pragma once

#include <cstdint>

struct Color
{
	Color() {}
	Color(uint8_t r, uint8_t g, uint8_t b) : r(r), g(g), b(b) {}

	uint8_t r = 0;
	uint8_t g = 0;
	uint8_t b = 0;
};

uint8_t lerp_u8(uint8_t a, uint8_t b, uint8_t factor);
Color blend(const Color& a, const Color& b, uint8_t factor);
