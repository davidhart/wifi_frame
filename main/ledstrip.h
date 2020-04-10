#pragma once

#include <driver/rmt.h>

class LedStripTimings
{
public:
	constexpr LedStripTimings(int T0H_ns, int T0L_ns, int T1H_ns, int T1L_ns, int reset_ns) :
		T0H_ns(T0H_ns),
		T0L_ns(T0L_ns),
		T1H_ns(T1H_ns),
		T1L_ns(T1L_ns),
		reset_ns(reset_ns)
	{
	}

	int T0H_ns;
	int T0L_ns;
	int T1H_ns;
	int T1L_ns;
	int reset_ns;

	
};

constexpr LedStripTimings Timings_WS2812b(400, 850, 850, 400, 100000);

class LedStrip
{
public:
	LedStrip(rmt_channel_t channel, gpio_num_t ledPin, int ledCount, const LedStripTimings& timings);
	~LedStrip();

	void setPixel(int pixelIndex, int red, int green, int blue);
	void refresh();

	inline int getLedCount() const { return m_ledCount; }

private:

	void setByte(int index, uint8_t byte);

	rmt_channel_t m_channel;
	gpio_num_t m_ledPin;
	int m_ledCount;

	rmt_item32_t m_itemBit0;
	rmt_item32_t m_itemBit1;
	rmt_item32_t m_itemReset;

	rmt_item32_t* m_data;
	int m_dataSize;
};
