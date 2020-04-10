#include "color.h"

uint8_t lerp_u8(uint8_t a, uint8_t b, uint8_t factor)
{
	uint32_t val = (uint32_t)a*(256-factor) + (uint32_t)b*factor;
	return (uint8_t)(val / 256);
}

Color blend(const Color& a, const Color& b, uint8_t factor)
{
	return Color(
		lerp_u8(a.r, b.r, factor),
		lerp_u8(a.g, b.g, factor),
		lerp_u8(a.b, b.b, factor)
	);
}
