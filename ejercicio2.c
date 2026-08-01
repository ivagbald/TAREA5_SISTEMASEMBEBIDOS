#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/adc.h"

#define LED_PIN GPIO_NUM_2
// En el ESP32, el GPIO34 corresponde al canal 6 del ADC1
#define SENSOR_PIN ADC1_CHANNEL_6 

int sensorValue = 0;

// Prototipos de tareas
void TaskReadSensor(void *pvParameters);
void TaskBlinkLED(void *pvParameters);
void TaskSerialPrint(void *pvParameters);

void app_main() {
    // 1. Configurar LED
    gpio_reset_pin(LED_PIN);
    gpio_set_direction(LED_PIN, GPIO_MODE_OUTPUT);
    
    // 2. Configurar ADC (Sensor Virtual / Potenciómetro)
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(SENSOR_PIN, ADC_ATTEN_DB_11);
    
    // 3. Crear tareas de FreeRTOS con distintas prioridades
    // Tarea 1: Lectura periódica (Prioridad Alta = 2)
    xTaskCreate(TaskReadSensor, "LecturaSensor", 2048, NULL, 2, NULL);
    
    // Tarea 2: Control de LED (Prioridad Media = 1)
    xTaskCreate(TaskBlinkLED, "ControlLED", 2048, NULL, 1, NULL);
    
    // Tarea 3: Envío de información (Prioridad Baja = 0)
    xTaskCreate(TaskSerialPrint, "ImpresionSerial", 2048, NULL, 0, NULL);
}

void TaskReadSensor(void *pvParameters) {
    for (;;) {
        // Leer el valor crudo del ADC
        sensorValue = adc1_get_raw(SENSOR_PIN);
        // Esperar 100ms sin bloquear el sistema
        vTaskDelay(pdMS_TO_TICKS(100)); 
    }
}

void TaskBlinkLED(void *pvParameters) {
    int led_state = 0;
    for (;;) {
        led_state = !led_state; // Invertir el estado
        gpio_set_level(LED_PIN, led_state);
        // Esperar 500ms
        vTaskDelay(pdMS_TO_TICKS(500)); 
    }
}

void TaskSerialPrint(void *pvParameters) {
    for (;;) {
        // Imprimir el valor por consola (Monitor Serial)
        printf("Sensor Virtual (GPIO34): %d\n", sensorValue);
        // Esperar 1000ms
        vTaskDelay(pdMS_TO_TICKS(1000)); 
    }
}