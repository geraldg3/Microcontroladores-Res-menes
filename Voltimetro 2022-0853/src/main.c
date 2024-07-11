#include <stdio.h>
#include <math.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <driver/gpio.h>
#include <driver/adc.h>
#include <driver/ledc.h>

// Configuración del canal ADC
adc1_channel_t adc_pot = ADC1_CHANNEL_6;
#define LED 2

// Declaración de variables globales
float Obj = 18;
float voltajeRMS;
float Vmedido;
float voltajes[100];
int i = 0;

// Prototipo de la función de inicialización de hardware
void init_hw(void);

void app_main() {
    // Inicializa el hardware
    init_hw();

    while (1) {
        // Adquisición de 100 muestras de voltaje
        for (i = 0; i < 100; i++) {
            int pot = adc1_get_raw(adc_pot); // Obtiene la lectura bruta del ADC
            Vmedido = (pot * Obj / 4095); // Calcula el voltaje medido
            voltajes[i] = Vmedido; // Almacena el voltaje medido en el arreglo
            vTaskDelay(1 / portTICK_PERIOD_MS); // Espera 1 ms entre lecturas
        }

        // Cálculo del voltaje RMS
        for (size_t j = 0; j < 100; j++) {
            voltajes[j] = pow(voltajes[j], 2); // Eleva al cuadrado cada muestra de voltaje
            if (j > 0) {
                voltajes[j] += voltajes[j - 1]; // Acumula los valores cuadrados
            }
        }

        float vtemp = voltajes[99] / 100; // Promedia los valores cuadrados
        vtemp = sqrt(vtemp); // Calcula la raíz cuadrada del promedio para obtener el RMS
        printf("Voltaje: %4.2fV\n", vtemp); // Imprime el voltaje RMS

        vTaskDelay(1000 / portTICK_PERIOD_MS); // Espera 1 segundo antes de repetir el ciclo
    }
}

// Función para inicializar el hardware
void init_hw(void) {
    // Configuración del ADC
    adc1_config_width(ADC_WIDTH_BIT_12); // Configura la resolución del ADC a 12 bits
    adc1_config_channel_atten(adc_pot, ADC_ATTEN_DB_12); // Configura la atenuación del canal ADC

    // Configuración del GPIO para el LED
    gpio_config_t io_config;
    io_config.mode = GPIO_MODE_OUTPUT; // Modo de salida
    io_config.pin_bit_mask = (1 << LED); // Máscara de pines (LED en el pin 2)
    io_config.pull_down_en = GPIO_PULLDOWN_ONLY; // Habilita la resistencia de pull-down
    io_config.pull_up_en = GPIO_PULLUP_DISABLE; // Deshabilita la resistencia de pull-up
    io_config.intr_type = GPIO_INTR_DISABLE; // Deshabilita las interrupciones
    gpio_config(&io_config); // Aplica la configuración del GPIO
}