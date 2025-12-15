#include <esp_log.h>
#include "task_shared.h"

static const char* TAG = "BleTask";

void ble_task(void* pvParameters)
{
    ESP_LOGI(TAG, "BLE task started");
    bleTarget.begin();

    while (1)
    {
        bleTarget.update();

        uint32_t message = 0;
        while (bleTarget.fetchMessage(&message, 0))
        {
            if (xSemaphoreTake(statsMutex, portMAX_DELAY) == pdTRUE)
            {
                all_expected_messages++;
                expectedMessage = message;
                hasExpectedMessage = true;
                last_expected_update = xTaskGetTickCount();
                xSemaphoreGive(statsMutex);
            }
            ESP_LOGI(TAG, "[BLE] %lu ms | Expected Data: %u", pdTICKS_TO_MS(xTaskGetTickCount()), (unsigned)expectedMessage);
        }

        vTaskDelay(pdMS_TO_TICKS(10));
    }
}
