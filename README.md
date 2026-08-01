Guía de Simulación - ESP32 (ESP-IDF)
Este repositorio contiene la documentación completa, diagramas y códigos fuente en C puro (ESP-IDF) para los tres ejercicios de sistemas embebidos utilizando el simulador Wokwi.
---
Índice
Ejercicio 1: Consola de Comandos por UART
Ejercicio 2: Tareas Concurrentes y Prioridades (FreeRTOS)
Ejercicio 3: Sincronización por Colas y Driver I2C (OLED)
Instrucciones Generales de Simulación en Wokwi
---
Ejercicio 1: Consola de Comandos por UART
Descripción
Implementación de una interfaz de consola interactiva basada en el periférico UART0 del ESP32 para recibir comandos del usuario a través del teclado virtual y controlar actuadores físicos.
Explicación del Funcionamiento
El sistema configura el puerto UART0 a 115200 baudios y un pin GPIO como salida digital. Utiliza un bucle de lectura no bloqueante para recolectar caracteres. Al presionar Enter, evalúa la cadena ingresada (`led on`, `led off`, `status`), ejecuta la acción correspondiente y responde por la misma consola.
Código de Referencia: ejercicio1.c

---
Ejercicio 2: Tareas Concurrentes y Prioridades (FreeRTOS)
Descripción
Desarrollo de un sistema multitarea en tiempo real empleando FreeRTOS en ESP-IDF. Permite la lectura periódica de un sensor analógico (ADC), el parpadeo de un LED y el envío de trazas por consola con diferentes prioridades.
Explicación del Funcionamiento
Se crean tres tareas independientes:
Tarea 1 (Prioridad Alta - 2): Lee el sensor analógico (GPIO34 / ADC1_CHANNEL_6) cada 100ms.
Tarea 2 (Prioridad Media - 1): Conmuta el estado del LED (GPIO2) cada 500ms.
Tarea 3 (Prioridad Baja - 0): Envía el valor del sensor por consola mediante `printf` cada 1000ms.
Todas usan `vTaskDelay` para ceder tiempo de CPU eficazmente.
Código de Referencia: ejercicio2.c

---
Ejercicio 3: Sincronización por Colas y Driver I2C (OLED)
Descripción
Sistema avanzado que combina comunicación asíncrona por UART, colas de mensajes (`xQueue`) en FreeRTOS, y un driver I2C minimalista escrito desde cero para visualizar líneas de tiempo de ejecución en una pantalla OLED SSD1306.
Explicación del Funcionamiento
Cola de Mensajes: La tarea UART recibe comandos (`cmd1`, `cmd0`) y los encola de manera segura. La tarea de acción extrae los comandos de la cola para gobernar el LED.
Driver I2C Nativo: Configura el bus maestro I2C en los pines GPIO21 (SDA) y GPIO22 (SCL). Envía comandos de inicialización y tramas de bytes (`framebuffer`) directamente a la memoria de video de la pantalla OLED para dibujar gráficas temporales en tiempo real.
Código de Referencia: ejercicio3.c

---
Instrucciones Generales de Simulación en Wokwi
Creación del Proyecto: Inicia un nuevo proyecto ESP32 seleccionando explícitamente el framework ESP-IDF.
Editor de Código: Reemplaza el archivo `main/src/main.c` (o `main.c`) con el código correspondiente al ejercicio.
Consola y Foco:
Haz clic en el botón verde de ejecución (Play).
Haz clic con el ratón directamente sobre la ventana negra de la consola inferior para darle el foco de entrada.
Escribe los comandos (ej. `cmd1`, `led on`) y presiona Enter. (Nota: Wokwi no muestra eco de caracteres en pantalla, se escriben a ciegas).
Periféricos: Añade elementos visuales desde el archivo `diagram.json` o la interfaz de Wokwi (LEDs, potenciómetro en GPIO34, pantalla OLED en SDA:21 / SCL:22).
