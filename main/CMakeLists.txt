idf_component_register(SRCS "main.cpp" "ledstrip.cpp" "color.cpp" "colorcurve.cpp" "sunclock.cpp" "led_strip_encoder.c"
     PRIV_REQUIRES esp_wifi esp_driver_rmt esp_http_server nvs_flash protocol_examples_common esp_driver_gpio
     INCLUDE_DIRS ".")
    