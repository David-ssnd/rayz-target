#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/semphr.h>
#include <freertos/task.h>

#include "ble_target.h"
#include "config.h"
#include "hash.h"
#include "photodiode.hpp"
#include "utils.h"
#include "wifi_manager.h"
#include <driver/gpio.h>
#include <esp_log.h>

static const char* TAG = "Target";

Photodiode photodiode;
BLETarget bleTarget;

uint16_t expectedMessage = 0;
bool hasExpectedMessage = false;

uint16_t all_expected_messages = 0;
uint16_t correct_messages = 0;
uint16_t not_expected_messages = 0;

// FreeRTOS queues and synchronization
QueueHandle_t photodiodeMessageQueue;
SemaphoreHandle_t statsMutex;

void photodiode_task(void* pvParameters)
{
    ESP_LOGI(TAG, "Photodiode task started");

    TickType_t lastWake = xTaskGetTickCount();
    const TickType_t samplePeriodTicks = pdMS_TO_TICKS(SAMPLE_INTERVAL_MS);

    while (1)
    {
        photodiode.update();

        if (photodiode.isSampleBufferFull())
        {
            uint16_t message16bit = photodiode.convertToBits();

            // Send to processing queue
            if (xQueueSend(photodiodeMessageQueue, &message16bit, 0) != pdTRUE)
            {
                ESP_LOGW(TAG, "Failed to send to photodiode queue");
            }
        }

        vTaskDelayUntil(&lastWake, samplePeriodTicks);
    }
}

void ble_task(void* pvParameters)
{
    ESP_LOGI(TAG, "BLE task started");

    bleTarget.begin();

    while (1)
    {
        bleTarget.update();

        uint16_t message = 0;
        while (bleTarget.fetchMessage(&message, 0))
        {
            if (xSemaphoreTake(statsMutex, portMAX_DELAY) == pdTRUE)
            {
                all_expected_messages++;
                expectedMessage = message;
                hasExpectedMessage = true;
                xSemaphoreGive(statsMutex);
            }

            ESP_LOGI(TAG, "[BLE] %lu ms | %s | Expected Data: %u", pdTICKS_TO_MS(xTaskGetTickCount()),
                     toBinaryString(expectedMessage, MESSAGE_TOTAL_BITS).c_str(), expectedMessage & 0xFF);
        }

        vTaskDelay(pdMS_TO_TICKS(10)); // 10ms delay
    }
}
void processing_task(void* pvParameters)
{
    ESP_LOGI(TAG, "Processing task started");
    uint16_t message16bit;

    while (1)
    {
        if (xQueueReceive(photodiodeMessageQueue, &message16bit, portMAX_DELAY) == pdTRUE)
        {
            uint16_t dataValue = 0;
            bool isValid = validateMessage16bit(message16bit, &dataValue);

            if (!isValid)
            {
                continue;
            }

            bool matchesBLE = false;
            uint16_t expectedSnapshot = 0;
            uint16_t correctSnapshot = correct_messages;
            uint16_t allSnapshot = all_expected_messages;
            uint16_t notExpectedSnapshot = not_expected_messages;
            float accuracy = 0.0f;

            if (xSemaphoreTake(statsMutex, portMAX_DELAY) == pdTRUE)
            {
                if (hasExpectedMessage && bleTarget.isConnected())
                {
                    matchesBLE = (message16bit == expectedMessage);
                    if (matchesBLE)
                    {
                        correct_messages++;
                    }
                    else
                    {
                        not_expected_messages++;
                    }
                }

                expectedSnapshot = expectedMessage;
                correctSnapshot = correct_messages;
                allSnapshot = all_expected_messages;
                notExpectedSnapshot = not_expected_messages;
                accuracy = (allSnapshot > 0) ? (correctSnapshot * 100.0f / allSnapshot) : 0.0f;

                xSemaphoreGive(statsMutex);
            }

            ESP_LOGI(TAG, "[Laser] %lu ms | %s | Data: %u (0x%X) | Sig: %.3fV | Thr: %.4fV | %s",
                     pdTICKS_TO_MS(xTaskGetTickCount()), toBinaryString(message16bit, 16).c_str(), dataValue, dataValue,
                     photodiode.getSignalStrength(), photodiode.getDynamicThreshold(),
                     matchesBLE ? "BLE MATCH" : "BLE MISMATCH");

            ESP_LOGI(TAG, "Stats: %u/%u | Incorrect: %u | Not Expected: %u | Accuracy: %.2f%%", correctSnapshot,
                     allSnapshot, allSnapshot - correctSnapshot, notExpectedSnapshot, accuracy);
        }
    }
}

extern "C" void app_main(void)
{
    ESP_LOGI(TAG, "Target device starting...");

    // Start WiFi provisioning / connection (non-blocking)
    wifi_manager_init("rayz-target", "target");

    // Initialize GPIO
    gpio_config_t io_conf = {};
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = (1ULL << VIBRATION_PIN);
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    gpio_config(&io_conf);
    gpio_set_level((gpio_num_t)VIBRATION_PIN, 0);

    // Initialize photodiode
    photodiode.begin();

    // Create queues and synchronization primitives
    photodiodeMessageQueue = xQueueCreate(10, sizeof(uint16_t));
    statsMutex = xSemaphoreCreateMutex();

    if (photodiodeMessageQueue == NULL || statsMutex == NULL)
    {
        ESP_LOGE(TAG, "Failed to create queues or mutex");
        return;
    }

    ESP_LOGI(TAG, "Target device ready - waiting for signals...");

    // Create tasks (WiFi runs independently)
    xTaskCreate(photodiode_task, "photodiode", 4096, NULL, 5, NULL);
    xTaskCreate(ble_task, "ble", 8192, NULL, 4, NULL);
    xTaskCreate(processing_task, "processing", 4096, NULL, 3, NULL);
}
