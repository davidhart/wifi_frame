#pragma once

#include <driver/rmt_tx.h>
#include <cstdint>

// in nanoseconds
struct led_strip_timings
{
	uint16_t bit0_h;
	uint16_t bit0_l;
	uint16_t bit1_h;
	uint16_t bit1_l;
	uint16_t reset;
};

constexpr led_strip_timings Timings_WS2812b = {
	.bit0_h = 400, 
	.bit0_l = 850,
	.bit1_h = 850, 
	.bit1_l = 400,
	.reset = 20000
};

class LedStrip
{
public:
	LedStrip(gpio_num_t ledPin, int ledCount, const led_strip_timings& timings);
	~LedStrip();

	void setPixel(int pixelIndex, int red, int green, int blue);
	void setPixel(int pixelIndex, uint8_t red, uint8_t green, uint8_t blue);
	void refresh();

	inline int getLedCount() const { return m_ledCount; }

private:

	rmt_channel_handle_t m_channel;
	rmt_encoder_handle_t m_encoder;
	gpio_num_t m_ledPin;
	int m_ledCount;

	uint8_t* m_data;
	int m_dataSize;
};
