#include "ledstrip.h"
#include "freertos/FreeRTOS.h"
#include "led_strip_encoder.h"

#define LOG_LOCAL_LEVEL ESP_LOG_VERBOSE
#include "esp_log.h"

#define LED_STRIP_CLK_RESOLUTION_HZ 10000000

const int BYTES_PER_PIXEL = 3;

static const char* TAG = "LedStrip";

LedStrip::LedStrip(gpio_num_t ledPin, int ledCount, const led_strip_timings& timings) :
	m_ledPin(ledPin),
	m_ledCount(ledCount)
{
	rmt_tx_channel_config_t config = {
        .gpio_num = ledPin,
        .clk_src = RMT_CLK_SRC_DEFAULT,
        .resolution_hz = LED_STRIP_CLK_RESOLUTION_HZ,
        .mem_block_symbols = 64,
        .trans_queue_depth = 4,
        .intr_priority = 0,
        .flags = { 
            .invert_out = 0,
            .with_dma = 0,
            .allow_pd = 0,
            .init_level = 0
        }
    };
    ESP_ERROR_CHECK(rmt_new_tx_channel(&config, &m_channel));

    led_strip_encoder_config_t encoder_config = {
        .resolution = LED_STRIP_CLK_RESOLUTION_HZ,
    };
    ESP_ERROR_CHECK(rmt_new_led_strip_encoder(&encoder_config, &m_encoder));

    
    ESP_ERROR_CHECK(rmt_enable(m_channel));

    m_dataSize = ledCount * BYTES_PER_PIXEL;
    m_data = new uint8_t[m_dataSize];
}

LedStrip::~LedStrip()
{
    delete[] m_data;
	ESP_ERROR_CHECK(rmt_del_channel(m_channel));
}

void LedStrip::setPixel(int pixelIndex, int red, int green, int blue)
{
	int i = pixelIndex * BYTES_PER_PIXEL;

	// Byte order grb
	m_data[i] = (uint8_t)(green & 0xFF);
	m_data[i+1] = (uint8_t)(red & 0xFF);
	m_data[i+2] = (uint8_t)(blue & 0xFF);
}

void LedStrip::setPixel(int pixelIndex, uint8_t red, uint8_t green, uint8_t blue)
{
    int i = pixelIndex * BYTES_PER_PIXEL;

    // Byte order grb
    m_data[i] = green;
    m_data[i+1] = red;
    m_data[i+2] = blue;   
}

void LedStrip::refresh()
{
    rmt_transmit_config_t tx_config = {
        .loop_count = 0, // no transfer loop
        .flags {
            .eot_level = 0,
            .queue_nonblocking = 0
        }
    };

    ESP_ERROR_CHECK(rmt_transmit(m_channel, m_encoder, m_data, m_dataSize, &tx_config));
    ESP_ERROR_CHECK(rmt_tx_wait_all_done(m_channel, portMAX_DELAY));
}
