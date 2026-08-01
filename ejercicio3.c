#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include "driver/i2c.h"

// Configuraciones UART0 (Para consola Wokwi)
#define UART_PORT UART_NUM_0
#define BUF_SIZE 1024

// Configuraciones I2C y OLED
#define I2C_MASTER_SCL_IO 22
#define I2C_MASTER_SDA_IO 21
#define I2C_MASTER_NUM I2C_NUM_0
#define I2C_MASTER_FREQ_HZ 400000
#define OLED_ADDR 0x3C

#define LED_PIN GPIO_NUM_2

QueueHandle_t commandQueue;
int t_uart_x = 0;
int t_action_x = 0;

// Memoria de video simplificada para la pantalla (8 páginas de 128 columnas)
uint8_t framebuffer[8][128];

// ==========================================
// DRIVER MINIMALISTA OLED SSD1306 (I2C)
// ==========================================
void oled_send_cmd(uint8_t cmd) {
    uint8_t data[2] = {0x00, cmd}; // 0x00 indica que enviamos un comando
    i2c_master_write_to_device(I2C_MASTER_NUM, OLED_ADDR, data, 2, 1000 / portTICK_PERIOD_MS);
}

void oled_init() {
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_MASTER_FREQ_HZ,
    };
    i2c_param_config(I2C_MASTER_NUM, &conf);
    i2c_driver_install(I2C_MASTER_NUM, conf.mode, 0, 0, 0);

    // Secuencia de inicialización básica del SSD1306
    uint8_t init_cmds[] = {
        0xAE, // Display OFF
        0x20, 0x00, // Modo de direccionamiento horizontal
        0x21, 0, 127, // Columnas (0-127)
        0x22, 0, 7,   // Páginas (0-7)
        0x81, 0xCF, // Contraste
        0xA1, // Remapeo de segmentos (Invertir X)
        0xC8, // Dirección de escaneo (Invertir Y)
        0xA6, // Display normal (No invertido)
        0xAF  // Display ON
    };
    for (int i = 0; i < sizeof(init_cmds); i++) {
        oled_send_cmd(init_cmds[i]);
    }
}

void oled_update() {
    uint8_t data[129];
    for (int page = 0; page < 8; page++) {
        oled_send_cmd(0xB0 + page); // Seleccionar página
        oled_send_cmd(0x00); // Columna baja
        oled_send_cmd(0x10); // Columna alta
        
        data[0] = 0x40; // 0x40 indica que enviamos datos (píxeles)
        memcpy(&data[1], framebuffer[page], 128);
        i2c_master_write_to_device(I2C_MASTER_NUM, OLED_ADDR, data, 129, 1000 / portTICK_PERIOD_MS);
    }
}

void draw_timeline(int page, int x_pos) {
    memset(framebuffer[page], 0, 128); // Limpiar la línea completa
    for (int i = 0; i < x_pos; i++) {
        framebuffer[page][i] = 0x08; // Dibujar línea horizontal
    }
    framebuffer[page][x_pos] = 0x3E; // Dibujar "cabeza" de la línea (círculo)
}

// ==========================================
// TAREAS FREERTOS
// ==========================================
void TaskUART(void *pvParameters) {
    uint8_t data_buf[128];
    for (;;) {
        // Lectura UART no bloqueante
        int len = uart_read_bytes(UART_PORT, data_buf, sizeof(data_buf) - 1, 10 / portTICK_PERIOD_MS);
        if (len > 0) {
            data_buf[len] = '\0';
            char* cmd_str = (char*)data_buf;
            cmd_str[strcspn(cmd_str, "\r\n")] = 0; 
            
            int cmd_val = -1;
            if (strcmp(cmd_str, "cmd1") == 0) cmd_val = 1;
            else if (strcmp(cmd_str, "cmd0") == 0) cmd_val = 0;
            
            if (cmd_val != -1) {
                // Enviar a la cola
                xQueueSend(commandQueue, &cmd_val, portMAX_DELAY);
                printf("Comando %s recibido y encolado.\n", cmd_str);
            }
        }
        
        t_uart_x = (t_uart_x + 1) % 127; 
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

void TaskAction(void *pvParameters) {
    int received_cmd;
    for (;;) {
        // Escuchar la cola esperando hasta 10ms
        if (xQueueReceive(commandQueue, &received_cmd, pdMS_TO_TICKS(10)) == pdPASS) {
            gpio_set_level(LED_PIN, received_cmd);
            printf("Accion Ejecutada: LED %s\n", received_cmd ? "ENCENDIDO" : "APAGADO");
        }
        
        t_action_x = (t_action_x + 2) % 127;
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void TaskDisplay(void *pvParameters) {
    for (;;) {
        // Dibujamos las líneas de tiempo en diferentes "páginas" de la pantalla OLED
        // Página 2 (aprox Y=16) para UART
        draw_timeline(2, t_uart_x); 
        // Página 5 (aprox Y=40) para la Acción
        draw_timeline(5, t_action_x); 
        
        oled_update(); // Refrescar pantalla
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

// ==========================================
// FUNCIÓN PRINCIPAL
// ==========================================
void app_main() {
    // Configurar LED
    gpio_reset_pin(LED_PIN);
    gpio_set_direction(LED_PIN, GPIO_MODE_OUTPUT);

    // Configurar UART0
    uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };
    uart_param_config(UART_PORT, &uart_config);
    uart_driver_install(UART_PORT, BUF_SIZE * 2, 0, 0, NULL, 0);

    // Inicializar Pantalla
    oled_init();

    // Crear cola de comunicación
    commandQueue = xQueueCreate(5, sizeof(int));

    // Crear tareas
    xTaskCreate(TaskUART, "UART_Cmd", 4096, NULL, 1, NULL);
    xTaskCreate(TaskAction, "Action_Exec", 2048, NULL, 2, NULL);
    xTaskCreate(TaskDisplay, "OLED_Draw", 4096, NULL, 1, NULL);
}