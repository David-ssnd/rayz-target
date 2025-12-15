#include <freertos/FreeRTOS.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include <driver/gpio.h>
#include <esp_log.h>
#include "config.h"
#include "game_protocol.h"
#include "game_state.h"
#include "task_shared.h"
#include "tasks.h"
#include "wifi_manager.h"

static const char* TAG = "Target";

extern "C" void app_main(void)
{
    ESP_LOGI("Target", "=== RayZ Target Starting ===");

    if (!game_state_init(DEVICE_ROLE_TARGET))
    {
        ESP_LOGE(TAG, "Failed to initialize game state");
        return;
    }
    ESP_LOGI("Target", "Game state initialized - Device ID: %u", game_state_get_config()->device_id);

    wifi_manager_init("rayz-target", "target");

    gpio_config_t io_conf = {};
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = (1ULL << VIBRATION_PIN);
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    gpio_config(&io_conf);
    gpio_set_level((gpio_num_t)VIBRATION_PIN, 0);

    photodiode.begin();

    if (!init_task_shared())
    {
        ESP_LOGE(TAG, "Failed to create queues or mutex");
        return;
    }

    ESP_LOGI(TAG, "Target device ready - waiting for signals...");

    xTaskCreate(photodiode_task, "photodiode", 4096, NULL, 5, NULL);
    xTaskCreate(processing_task, "processing", 4096, NULL, 3, NULL);
    xTaskCreate(espnow_task, "espnow", 4096, NULL, 3, NULL);
    xTaskCreate(ws_task, "websocket", 8192, NULL, 2, NULL);

    ESP_LOGI(TAG, "All tasks created successfully");
}
