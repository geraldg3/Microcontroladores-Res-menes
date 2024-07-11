// Librerías necesarias para el programa
#include <stdio.h> // Librería estándar de entrada/salida en C
#include <stdlib.h>
#include <freertos/FreeRTOS.h> // Librería FreeRTOS para el manejo de delays
#include <freertos/task.h>
#include <driver/gpio.h> // Librería para el manejo de GPIO
#include <freertos/timers.h> // Librería para el uso de timers
#include "esp_log.h" // Librería para el logging de ESP-IDF

// Definiciones de constantes y variables
static const char *tag = "Main";

#define TRUE 1
#define FALSE 0

// Estados posibles del sistema
#define ESTADO_INIT         0
#define ESTADO_ABRIENDO     1
#define ESTADO_CERRANDO     2
#define ESTADO_CERRADO      3
#define ESTADO_ABIERTO      4
#define ESTADO_EMERGENCIA   5
#define ESTADO_ERROR        6
#define ESTADO_ESPERA       7

// Definición del handle del timer y otros parámetros
TimerHandle_t xTimers;
int timerID = 1;
int INTERVALO = 50; // Intervalo del timer en milisegundos

// Declaración de funciones
esp_err_t INTERRUPCION_50MS(void); // Función para manejar interrupciones de 50ms
esp_err_t SET_TIMER(void); // Función para configurar el timer

// Declaración de funciones de estado
int Func_ESTADO_INIT();
int Func_ESTADO_ABRIENDO();
int Func_ESTADO_CERRANDO();
int Func_ESTADO_CERRADO();
int Func_ESTADO_ABIERTO();
int Func_ESTADO_EMERGENCIA();
int Func_ESTADO_ERROR();
int Func_ESTADO_ESPERA();

// Definición de las estructuras de entradas y salidas
volatile struct INPUTS {
    unsigned int LSA: 1; // Limit Switch de abierta
    unsigned int LSC: 1; // Limit Switch de cerrada
    unsigned int CA:  1; // Comando de abrir
    unsigned int CC:  1; // Comando de cerrar
    unsigned int FC:  1; // Sensor de obstrucción
} inputs;

volatile struct OUTPUTS {
    unsigned int MC: 1; // Motor cerrado
    unsigned int MA: 1; // Motor abierto
    unsigned int LED_EMERGENCIA: 1; // LED de emergencia
    unsigned int LED_MOVIMIENTO: 1; // LED de movimiento
} outputs;

// Variables de estado del sistema
volatile int ESTADO_ACTUAL = ESTADO_INIT;
volatile int ESTADO_SIGUIENTE = ESTADO_INIT;
volatile int ESTADO_ANTERIOR = ESTADO_INIT;

/*******************/

// Función de callback para el timer
void vTimerCallback(TimerHandle_t pxTimer) {
    ESP_LOGE(tag, "Interrupción de 50ms activada.");
    INTERRUPCION_50MS();
}

// Función principal del programa
void app_main() {
    ESTADO_SIGUIENTE = Func_ESTADO_INIT();
    SET_TIMER();

    for (;;) {
        switch (ESTADO_SIGUIENTE) {
            case ESTADO_INIT:
                ESTADO_SIGUIENTE = Func_ESTADO_INIT();
                break;
            case ESTADO_ESPERA:
                ESTADO_SIGUIENTE = Func_ESTADO_ESPERA();
                break;
            case ESTADO_ABRIENDO:
                ESTADO_SIGUIENTE = Func_ESTADO_ABRIENDO();
                break;
            case ESTADO_CERRANDO:
                ESTADO_SIGUIENTE = Func_ESTADO_CERRANDO();
                break;
            case ESTADO_CERRADO:
                ESTADO_SIGUIENTE = Func_ESTADO_CERRADO();
                break;
            case ESTADO_ABIERTO:
                ESTADO_SIGUIENTE = Func_ESTADO_ABIERTO();
                break;
            case ESTADO_EMERGENCIA:
                ESTADO_SIGUIENTE = Func_ESTADO_EMERGENCIA();
                break;
            case ESTADO_ERROR:
                ESTADO_SIGUIENTE = Func_ESTADO_ERROR();
                break;
        }
    }
}

// Función para el estado de inicialización
int Func_ESTADO_INIT() {
    ESP_LOGE(tag, "Iniciando el programa");

    ESTADO_ANTERIOR = ESTADO_ACTUAL;
    ESTADO_ACTUAL = ESTADO_INIT;

    ESP_LOGE(tag, "Configurando los pines");
    // Configuración de pines de entrada y salida
    gpio_config_t IO_CONFIG;

    // Configuración de las entradas
    IO_CONFIG.mode = GPIO_MODE_INPUT;
    IO_CONFIG.pin_bit_mask = (1 << 13) | (1 << 12) | (1 << 14) | (1 << 27) | (1 << 26);
    IO_CONFIG.pull_down_en = GPIO_PULLDOWN_ENABLE;
    IO_CONFIG.pull_up_en = GPIO_PULLUP_DISABLE;
    IO_CONFIG.intr_type = GPIO_INTR_DISABLE;
    gpio_config(&IO_CONFIG);

    // Configuración de las salidas
    IO_CONFIG.mode = GPIO_MODE_OUTPUT;
    IO_CONFIG.pin_bit_mask = (1 << 4) | (1 << 16) | (1 << 17) | (1 << 5);
    IO_CONFIG.pull_down_en = GPIO_PULLDOWN_DISABLE;
    IO_CONFIG.pull_up_en = GPIO_PULLUP_DISABLE;
    IO_CONFIG.intr_type = GPIO_INTR_DISABLE;
    gpio_config(&IO_CONFIG);

    for (;;) {
        return ESTADO_ESPERA;
    }
}

// Función para el estado de apertura
int Func_ESTADO_ABRIENDO() {
    printf("Abriendo la puerta");

    ESTADO_ANTERIOR = ESTADO_ACTUAL;
    ESTADO_ACTUAL = ESTADO_ABRIENDO;

    // Actualización de las salidas
    outputs.LED_MOVIMIENTO = TRUE;
    outputs.LED_EMERGENCIA = FALSE;
    outputs.MA = TRUE;
    outputs.MC = FALSE;

    for (;;) {
        if (inputs.LSA == TRUE) {
            return ESTADO_ABIERTO;
        }
        if (inputs.LSA == TRUE && inputs.LSC == TRUE) {
            return ESTADO_ERROR;
        }
        if (inputs.FC == TRUE) {
            return ESTADO_EMERGENCIA;
        }
        if (inputs.CC == TRUE) {
            return ESTADO_CERRANDO;
        }
    }
}

// Función para el estado de cierre
int Func_ESTADO_CERRANDO() {
    printf("Cerrando la puerta");

    ESTADO_ANTERIOR = ESTADO_ACTUAL;
    ESTADO_ACTUAL = ESTADO_CERRANDO;

    // Actualización de las salidas
    outputs.LED_MOVIMIENTO = TRUE;
    outputs.LED_EMERGENCIA = FALSE;
    outputs.MA = FALSE;
    outputs.MC = TRUE;

    for (;;) {
        if (inputs.LSC == TRUE) {
            return ESTADO_CERRADO;
        }
        if (inputs.LSA == TRUE && inputs.LSC == TRUE) {
            return ESTADO_ERROR;
        }
        if (inputs.FC == TRUE) {
            return ESTADO_EMERGENCIA;
        }
        if (inputs.CA == TRUE) {
            return ESTADO_ABRIENDO;
        }
    }
}

// Función para el estado de cerrado
int Func_ESTADO_CERRADO() {
    printf("Puerta cerrada");

    ESTADO_ANTERIOR = ESTADO_ACTUAL;
    ESTADO_ACTUAL = ESTADO_CERRADO;

    outputs.LED_MOVIMIENTO = FALSE;
    outputs.LED_EMERGENCIA = FALSE;
    outputs.MA = FALSE;
    outputs.MC = FALSE;

    for (;;) {
        return ESTADO_ESPERA;
    }
}

// Función para el estado de abierto
int Func_ESTADO_ABIERTO() {
    printf("Puerta abierta");

    ESTADO_ANTERIOR = ESTADO_ACTUAL;
    ESTADO_ACTUAL = ESTADO_ABIERTO;

    outputs.LED_MOVIMIENTO = FALSE;
    outputs.LED_EMERGENCIA = FALSE;
    outputs.MA = FALSE;
    outputs.MC = FALSE;

    for (;;) {
        return ESTADO_ESPERA;
    }
}

// Función para el estado de emergencia
int Func_ESTADO_EMERGENCIA() {
    printf("¡Emergencia!");

    ESTADO_ANTERIOR = ESTADO_ACTUAL;
    ESTADO_ACTUAL = ESTADO_EMERGENCIA;

    outputs.LED_MOVIMIENTO = FALSE;
    outputs.LED_EMERGENCIA = TRUE;
    outputs.MA = FALSE;
    outputs.MC = FALSE;

    for (;;) {
        vTaskDelay(1500 / portTICK_PERIOD_MS);
        if (inputs.FC == FALSE) {
            return ESTADO_ANTERIOR;
        }
    }
}

// Función para el estado de error
int Func_ESTADO_ERROR() {
    ESTADO_ANTERIOR = ESTADO_ACTUAL;
    ESTADO_ACTUAL = ESTADO_ERROR;

    outputs.LED_MOVIMIENTO = FALSE;
    outputs.LED_EMERGENCIA = TRUE;
    outputs.MA = FALSE;
    outputs.MC = FALSE;

    printf("\n¡Error!");
    printf("\n¡Error!");
    printf("\n¡Error!");

    for (;;) {
        vTaskDelay(500 / portTICK_PERIOD_MS);
        outputs.LED_EMERGENCIA = FALSE;
        vTaskDelay(500 / portTICK_PERIOD_MS);
        outputs.LED_EMERGENCIA = TRUE;
        vTaskDelay(500 / portTICK_PERIOD_MS);
        outputs.LED_EMERGENCIA = FALSE;
    }
}