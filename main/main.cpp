#include <nvs_flash.h>
#include <esp_wifi.h>
#include <esp_sntp.h>
#include <esp_http_server.h>
#include <esp_event.h>
#include <esp_log.h>
#include <esp_types.h>
#include <esp_err.h>

#include <protocol_examples_common.h>

#include "ledstrip.h"

const char* TAG = "restapi";
#define LED_STATUS_GPIO GPIO_NUM_2

#define NUM_COLORS 2
volatile int led_red[NUM_COLORS] = {255, 0};
volatile int led_green[NUM_COLORS] = { 0, 0};
volatile int led_blue[NUM_COLORS] = { 0, 128};

extern "C" {
  void app_main();
}

static esp_err_t light_color_get_handler(httpd_req_t* req);

static const httpd_uri_t lightcolor = {
    .uri       = "/lightcolor",
    .method    = HTTP_GET,
    .handler   = light_color_get_handler,
    .user_ctx  = nullptr
};

static esp_err_t light_color_get_handler(httpd_req_t* req)
{
    int r = 0;
    int g = 0;
    int b = 0;
    int c = 0;

    size_t buf_len = httpd_req_get_url_query_len(req) + 1;
    if (buf_len > 1) {
        char* buf = (char*)malloc(buf_len);
        if (httpd_req_get_url_query_str(req, buf, buf_len) == ESP_OK) {
            ESP_LOGI(TAG, "Found URL query => %s", buf);
            char param[32];
            /* Get value of expected key from query string */
            if (httpd_query_key_value(buf, "r", param, sizeof(param)) == ESP_OK) {

                r = atoi(param);
                ESP_LOGI(TAG, "Found URL query parameter => r=%d", r);
            }
            if (httpd_query_key_value(buf, "g", param, sizeof(param)) == ESP_OK) {

                g = atoi(param);
                ESP_LOGI(TAG, "Found URL query parameter => g=%d", g);
            }
            if (httpd_query_key_value(buf, "b", param, sizeof(param)) == ESP_OK) {

                b = atoi(param);
                ESP_LOGI(TAG, "Found URL query parameter => b=%d", b);
            }
            if (httpd_query_key_value(buf, "c", param, sizeof(param)) == ESP_OK) {

                c = atoi(param);
                if (c < 0 || c > NUM_COLORS) 
                    c = 0;
                ESP_LOGI(TAG, "Found URL query parameter => c=%d", c);
            }
               
        }
        free(buf);
    }

    led_red[c] = r;
    led_green[c] = g;
    led_blue[c] = b;

    const char* resp_str = "lightcolor OK";
    httpd_resp_send(req, resp_str, strlen(resp_str));

    return ESP_OK;
}

static httpd_handle_t start_webserver(void)
{
    httpd_handle_t server = 0;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    // Start the httpd server
    ESP_LOGI(TAG, "Starting server on port: '%d'", config.server_port);
    if (httpd_start(&server, &config) == ESP_OK) {
        // Set URI handlers
        ESP_LOGI(TAG, "Registering URI handlers");
        httpd_register_uri_handler(server, &lightcolor);

        // Status LED on
        gpio_set_level(LED_STATUS_GPIO, 1);

        return server;
    }

    ESP_LOGI(TAG, "Error starting server!");
    return 0;
}

static void stop_webserver(httpd_handle_t server)
{
    gpio_set_level(LED_STATUS_GPIO, 0);

    httpd_stop(server);
}

static void disconnect_handler(void* arg, esp_event_base_t event_base, 
                               int32_t event_id, void* event_data)
{
    httpd_handle_t* server = (httpd_handle_t*) arg;
    if (*server) {
        ESP_LOGI(TAG, "Stopping webserver");
        stop_webserver(*server);
        *server = nullptr;
    }
}

static void connect_handler(void* arg, esp_event_base_t event_base, 
                            int32_t event_id, void* event_data)
{
    httpd_handle_t* server = (httpd_handle_t*) arg;
    if (*server == nullptr) {
        ESP_LOGI(TAG, "Starting webserver");
        *server = start_webserver();
    }
}

void refresh_leds_task(void* pvParameters)
{
    const int LED_COUNT = 12*4;
    LedStrip ledstrip(RMT_CHANNEL_0, GPIO_NUM_18, LED_COUNT, Timings_WS2812b);

    int x = 0;

    while(true)
    {
        for (int i = 0; i < LED_COUNT; i++)
        {
            int c = (i >= x && i < x + LED_COUNT/2) ||
                (i >= x - LED_COUNT && i < x - LED_COUNT/2) ? 0 : 1;

            ledstrip.setPixel(i, led_red[c], led_green[c], led_blue[c]);
        }
        ledstrip.refresh();

        x = (x + 1) % LED_COUNT;

        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

void app_main(void)
{
    // Init onboard LED
    gpio_pad_select_gpio(LED_STATUS_GPIO);
    gpio_set_direction(LED_STATUS_GPIO, GPIO_MODE_OUTPUT);

    static httpd_handle_t server = 0;

    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    // Register event handlers to start http server on connect and stop it on disconnect. 
    // Handles initial connect and re-connecting
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &connect_handler, &server));
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &disconnect_handler, &server));

    /* This helper function configures Wi-Fi or Ethernet, as selected in menuconfig.
     * Read "Establishing Wi-Fi or Ethernet Connection" section in
     * examples/protocols/README.md for more information about this function.
     */
    ESP_ERROR_CHECK(example_connect());

    // Sync network time
    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    sntp_setservername(0, "pool.ntp.org");
    sntp_set_sync_mode(SNTP_SYNC_MODE_SMOOTH);
    sntp_init();
    // TODO: set timezone

    // Create pinned to core1 - Important! rmt_driver_install performed by LedStrip must be performed
    // on core1 else the interrupt handlers will be installed on core0, shared with wifi, which seems to interrupt led timings
    xTaskCreatePinnedToCore(&refresh_leds_task, "refresh_leds", 1000, nullptr, 5, nullptr, 1);
}
