#include "ledstrip.h"

const int BITS_PER_PIXEL = 24;

uint32_t nsToTick(int ns, float ratio)
{
	return (uint32_t)(ns * ratio);
}

LedStrip::LedStrip(rmt_channel_t channel, gpio_num_t ledPin, int ledCount, const LedStripTimings& timings) :
	m_channel(channel),
	m_ledPin(ledPin),
	m_ledCount(ledCount)
{
	rmt_config_t config = RMT_DEFAULT_CONFIG_TX(ledPin, channel);
    // set counter clock to 20MHz
    config.clk_div = 4;

    config.tx_config.loop_en              = false;
    config.tx_config.carrier_en           = false;
    config.tx_config.carrier_freq_hz      = 0;
    config.tx_config.carrier_duty_percent = 0;
    config.tx_config.carrier_level        = RMT_CARRIER_LEVEL_LOW;
    config.tx_config.idle_level           = RMT_IDLE_LEVEL_LOW;
    config.tx_config.idle_output_en       = true;

    ESP_ERROR_CHECK(rmt_config(&config));
    ESP_ERROR_CHECK(rmt_driver_install(channel, 0, 0));
    ESP_ERROR_CHECK(rmt_set_mem_pd(channel, false));

    uint32_t counter_clk_hz = 0;
    ESP_ERROR_CHECK(rmt_get_counter_clock(channel, &counter_clk_hz));
    // ns -> ticks
    float ratio = (float)counter_clk_hz / 1e9;

    m_itemBit0.duration0 = nsToTick(timings.T0H_ns, ratio);
    m_itemBit0.level0 = 1;
    m_itemBit0.duration1 = nsToTick(timings.T0L_ns, ratio);
    m_itemBit0.level1 = 0;

    m_itemBit1.duration0 = nsToTick(timings.T1H_ns, ratio);
    m_itemBit1.level0 = 1;
    m_itemBit1.duration1 = nsToTick(timings.T1L_ns, ratio);
    m_itemBit1.level1 = 0;

    m_itemReset.duration0 = nsToTick(timings.reset_ns, ratio);
    m_itemReset.level0 = 0;
    m_itemReset.duration1 = 0;
    m_itemReset.level1 = 0;

    m_dataSize = BITS_PER_PIXEL * ledCount + 1;
    m_data = new rmt_item32_t[m_dataSize];

    // Init all LED bits to 0
    for (int i = 0; i < m_dataSize - 1; i++)
    {
    	m_data[i] = m_itemBit0;
    }
    // Terminate with reset signal
    m_data[m_dataSize - 1] = m_itemReset;
}

LedStrip::~LedStrip()
{
	rmt_driver_uninstall(m_channel);
}

void LedStrip::setPixel(int pixelIndex, int red, int green, int blue)
{
	int i = pixelIndex * BITS_PER_PIXEL;

	// Byte order grb
	setByte(i, (uint8_t)(green & 0xFF));
	setByte(i+8, (uint8_t)(red & 0xFF));
	setByte(i+16, (uint8_t)(blue & 0xFF));
}

void LedStrip::setPixel(int pixelIndex, uint8_t red, uint8_t green, uint8_t blue)
{
    int i = pixelIndex * BITS_PER_PIXEL;

    // Byte order grb
    setByte(i, green);
    setByte(i+8, red);
    setByte(i+16, blue);   
}

void LedStrip::refresh()
{
	ESP_ERROR_CHECK(rmt_write_items(m_channel, m_data, m_dataSize, true));
}

void LedStrip::setByte(int index, uint8_t byte)
{
	for (int i = 0; i < 8; i++)
	{
		bool bit = byte & (1 << (7 - i));
		m_data[index + i] = bit ? m_itemBit1 : m_itemBit0;
	}
}