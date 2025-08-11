#include <stdio.h>           // Biblioteca padrão de entrada e saída
#include <math.h>
#include "pico/stdlib.h"     // Biblioteca padrão do Raspberry Pi Pico
#include "hardware/adc.h"
#include "hardware/uart.h"
#include "hardware/gpio.h"

#define UART_ID uart0
#define BAUD_RATE 115200

#define UART_TX_PIN 0
#define UART_RX_PIN 1
#define ADC_VOLT 27 // GPIO para o voltímetro
#define ADC_AMP 26 // GPIO para o amperímetro

int main()
{
    stdio_init_all(); // Inicializa a entrada e saída padrão

    // Inicializa o ADC
    adc_init();
    adc_gpio_init(ADC_VOLT); // GPIO 27 como entrada analógica
    adc_gpio_init(ADC_AMP);  // GPIO 26 como entrada analógica

    //Inicializa comunicação UART
    uart_init(UART_ID, BAUD_RATE);
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);

    float tensao, corrente;

    uint16_t leitura_tensao, leitura_corrente;

    char buffer_uart[64];

    while (true)
    {  
        tensao = 0.0;
        corrente = 0.0;

        //Calcula valor RMS a partir de 4000 leituras
        for (int i = 0; i < 4000; i++) {
            adc_select_input(1); // Tensão
            leitura_tensao = adc_read();

            adc_select_input(0); // Corrente
            leitura_corrente = adc_read();

            float tensao_ac = (float)leitura_tensao - 1585.0;
            float corrente_ac = (float)leitura_corrente - 1585.0;

            tensao += tensao_ac * tensao_ac;
            corrente += corrente_ac * corrente_ac;
            sleep_us(50);
        }

        // Calibação dos valores RMS
        tensao = sqrt(tensao / 4000.0) * 0.618;
        corrente = sqrt(corrente / 4000.0) * 0.024338;

        // Formata e envia os dados via UART
        snprintf(buffer_uart, sizeof(buffer_uart), "T%.2f C%.2f \n", tensao, corrente);
        uart_puts(UART_ID, buffer_uart);

        printf(buffer_uart);

        sleep_ms(500);

    }
}