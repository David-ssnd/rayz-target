#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/semphr.h>
#include <freertos/task.h>

#include <esp_log.h>
#include <driver/gpio.h>
#include "ble_target.h"
#include "config.h"
#include "game_protocol.h"
#include "game_state.h"
#include "hash.h"
#include "photodiode.hpp"
#include "utils.h"
#include "wifi_manager.h"
#include "ws_client.h"

static const char* TAG = "Target";

// WebSocket server URI - configure for your server
#define WS_SERVER_URI "ws://192.168.1.100:3000/ws"

Photodiode photodiode;
BLETarget bleTarget;

uint16_t expectedMessage = 0;
bool hasExpectedMessage = false;

uint16_t all_expected_messages = 0;
uint16_t correct_messages = 0;
uint16_t not_expected_messages = 0;

// FreeRTOS queues and synchronization
QueueHandle_t photodiodeMessageQueue;
QueueHandle_t hitEventQueue; // Queue for hit events to send to server
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

    const GameStateData* game_state = game_state_get();

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

            // Check if we're respawning - ignore hits during respawn
            if (game_state->state == GAME_STATE_RESPAWNING)
            {
                ESP_LOGD(TAG, "Ignoring hit - respawning");
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

            // === GAME HIT LOGIC ===
            // A valid laser hit detected! Report to server
            if (isValid && matchesBLE)
            {
                // Vibrate to indicate hit
                gpio_set_level((gpio_num_t)VIBRATION_PIN, 1);
                vTaskDelay(pdMS_TO_TICKS(VIBRATION_DURATION_MS));
                gpio_set_level((gpio_num_t)VIBRATION_PIN, 0);

                // Report hit to server
                // For now, we don't know the shooter ID from the laser message
                // In a real implementation, the message data would encode shooter info
                if (ws_client_is_connected())
                {
                    // TODO: Extract shooter ID from message data
                    ws_client_send_hit_report("unknown");
                    ESP_LOGI(TAG, "Hit reported to server!");
                }

                // Notify paired weapon via BLE that we got hit
                // (The weapon can then update its display)
            }
        }
    }
}

// WebSocket callbacks
static void on_ws_connect(bool connected)
{
    ESP_LOGI(TAG, "WebSocket %s", connected ? "connected" : "disconnected");
}

static void on_ws_hit(const char* shooter_id, const char* target_id, bool valid)
{
    ESP_LOGI(TAG, "Hit event: %s -> %s (%s)", shooter_id, target_id, valid ? "valid" : "invalid");

    // If we're the target and hit was valid, we died
    const DeviceConfig* config = game_state_get_config();
    if (valid && strcmp(config->player_id, target_id) == 0)
    {
        game_state_record_death();
        ESP_LOGI(TAG, "We were hit! Starting respawn...");

        // Vibrate to indicate death
        gpio_set_level((gpio_num_t)VIBRATION_PIN, 1);
        vTaskDelay(pdMS_TO_TICKS(500)); // Long vibration for death
        gpio_set_level((gpio_num_t)VIBRATION_PIN, 0);
    }
}

static void on_ws_game_state(GameMode mode, GameState state)
{
    ESP_LOGI(TAG, "Game state: mode=%s, state=%s", GAMEMODE_NAMES[mode], GAME_STATE_NAMES[state]);
}

void ws_task(void* pvParameters)
{
    ESP_LOGI(TAG, "WebSocket task started");

    // Wait for WiFi connection
    while (!wifi_manager_is_connected())
    {
        vTaskDelay(pdMS_TO_TICKS(500));
    }

    ESP_LOGI(TAG, "WiFi connected, initializing WebSocket client");

    // Generate default device name if not set
    DeviceConfig* config = game_state_get_config_mut();
    if (strlen(config->device_name) == 0)
    {
        const char* ip_str = wifi_manager_get_ip();
        game_state_generate_name(config->device_name, sizeof(config->device_name), config->role, ip_str);
        game_state_save_config();
        ESP_LOGI(TAG, "Generated device name: %s", config->device_name);
    }

    // Initialize WebSocket client
    WsClientConfig ws_config = {.server_uri = WS_SERVER_URI,
                                .on_connect = on_ws_connect,
                                .on_message = NULL,
                                .on_hit = on_ws_hit,
                                .on_game_state = on_ws_game_state,
                                .on_config = NULL};

    if (!ws_client_init(&ws_config))
    {
        ESP_LOGE(TAG, "Failed to initialize WebSocket client");
        vTaskDelete(NULL);
        return;
    }

    if (!ws_client_start())
    {
        ESP_LOGE(TAG, "Failed to start WebSocket client");
        vTaskDelete(NULL);
        return;
    }

    // Main loop
    while (1)
    {
        if (ws_client_is_connected())
        {
            if (game_state_heartbeat_due())
            {
                ws_client_send_heartbeat();
            }
            if (game_state_check_respawn())
            {
                ws_client_send_respawn_complete();
                ESP_LOGI(TAG, "Respawn complete!");
            }
        }
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

extern "C" void app_main(void)
{
    ESP_LOGI(TAG, "=== RayZ Target Starting ===");

    // Initialize game state manager first
    if (!game_state_init(DEVICE_ROLE_TARGET))
    {
        ESP_LOGE(TAG, "Failed to initialize game state");
        return;
    }
    ESP_LOGI(TAG, "Game state initialized - Device ID: %s", game_state_get_config()->device_id);

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
    hitEventQueue = xQueueCreate(5, sizeof(uint16_t));
    statsMutex = xSemaphoreCreateMutex();

    if (photodiodeMessageQueue == NULL || statsMutex == NULL || hitEventQueue == NULL)
    {
        ESP_LOGE(TAG, "Failed to create queues or mutex");
        return;
    }

    ESP_LOGI(TAG, "Target device ready - waiting for signals...");

    // Create tasks
    xTaskCreate(photodiode_task, "photodiode", 4096, NULL, 5, NULL);
    xTaskCreate(ble_task, "ble", 8192, NULL, 4, NULL);
    xTaskCreate(processing_task, "processing", 4096, NULL, 3, NULL);
    xTaskCreate(ws_task, "websocket", 8192, NULL, 2, NULL);

    ESP_LOGI(TAG, "All tasks created successfully");
}
