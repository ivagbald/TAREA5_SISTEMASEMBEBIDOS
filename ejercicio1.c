#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"
#include "driver/gpio.h"

// Configuración requerida para UART2
#define UART_PORT UART_NUM_0
#define TXD_PIN (GPIO_NUM_1)
#define RXD_PIN (GPIO_NUM_3)
#define LED_PIN (GPIO_NUM_2)
#define BUF_SIZE (1024)

int command_count = 0;

void app_main() {
    // Configurar LED
    gpio_reset_pin(LED_PIN);
    gpio_set_direction(LED_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level(LED_PIN, 0);

    // Configurar UART2
    uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };
    
    uart_param_config(UART_PORT, &uart_config);
    uart_set_pin(UART_PORT, TXD_PIN, RXD_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    uart_driver_install(UART_PORT, BUF_SIZE * 2, 0, 0, NULL, 0);

    uint8_t *data = (uint8_t *) malloc(BUF_SIZE);

    while (1) {
        // Lectura no bloqueante del buffer
        int len = uart_read_bytes(UART_PORT, data, BUF_SIZE - 1, 20 / portTICK_PERIOD_MS);
        if (len > 0) {
            data[len] = '\0';
            command_count++;
            
            char* cmd = (char*)data;
            cmd[strcspn(cmd, "\r\n")] = 0; // Limpiar saltos de línea

            // Interpretación de comandos obligatorios
            if (strcmp(cmd, "status") == 0) {
                uart_write_bytes(UART_PORT, "Estado: Sistema Operativo OK\r\n", 30);
            } else if (strcmp(cmd, "led on") == 0) {
                gpio_set_level(LED_PIN, 1);
                uart_write_bytes(UART_PORT, "Accion: LED Encendido\r\n", 23);
            } else if (strcmp(cmd, "led off") == 0) {
                gpio_set_level(LED_PIN, 0);
                uart_write_bytes(UART_PORT, "Accion: LED Apagado\r\n", 21);
            } else if (strcmp(cmd, "info") == 0) {
                char info_str[100];
                sprintf(info_str, "Info: Baud 115200, Puerto UART2, Comandos procesados: %d\r\n", command_count);
                uart_write_bytes(UART_PORT, info_str, strlen(info_str));
            } else if (strcmp(cmd, "reset") == 0) {
                command_count = 0;
                uart_write_bytes(UART_PORT, "Reset: Contadores reiniciados\r\n", 31);
            } else {
                uart_write_bytes(UART_PORT, "Error: Comando no reconocido\r\n", 30);
            }
        }
    }
}