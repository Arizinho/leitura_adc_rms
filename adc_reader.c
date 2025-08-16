#include <stdio.h>           // Biblioteca padrão de entrada e saída
#include <math.h>
#include "pico/stdlib.h"     // Biblioteca padrão do Raspberry Pi Pico
#include "hardware/adc.h"
#include "hardware/gpio.h"

#define ADC_VOLT 27 // GPIO para o voltímetro
#define ADC_AMP 26 // GPIO para o amperímetro

#define AMOSTRAS 256
#define ATRASO 10 

const float WC = 2.0f * M_PI * 106.0f;
const float A  = 2.0f * 7692.0f;
const float B  = WC * sqrtf(2.0f);
const float C  = WC * WC;

const float X1 = C / (A*A + A*B + C);
const float X2 = 2.0f * (C - A*A) / (A*A + A*B + C);
const float X3 = (A*A + C - A*B) / (A*A + A*B + C);

int main()
{
    stdio_init_all(); // Inicializa a entrada e saída padrão

    // Inicializa o ADC
    adc_init();
    adc_gpio_init(ADC_VOLT); // GPIO 27 como entrada analógica
    adc_gpio_init(ADC_AMP);  // GPIO 26 como entrada analógica

    float tensao, corrente;

    float leitura_tensao[AMOSTRAS], leitura_corrente[AMOSTRAS];
    float filtro_tensao[AMOSTRAS/2], filtro_corrente[AMOSTRAS/2];

    while (true)
    {  
        tensao = 0.0;
        corrente = 0.0;

        for (int i = 0; i < AMOSTRAS; i++) {
            adc_select_input(1); // Corrente
            leitura_tensao [i] = ((float)adc_read() - 1585.0)*0.6617;

            if(i>=ATRASO) {
                adc_select_input(0); // Corrente
                leitura_corrente[i-ATRASO] = (-(float)adc_read() + 1585.0)*0.021055;
            }

            

            // float tensao_ac = (float)leitura_tensao - 1585.0;
            // float corrente_ac = (float)leitura_corrente - 1585.0;

            // tensao += tensao_ac * tensao_ac;
            // corrente += corrente_ac * corrente_ac;
            sleep_us(130);
        }

        adc_select_input(0); // Corrente
        for (int i = AMOSTRAS; i < AMOSTRAS+ATRASO; i++) {
            leitura_corrente[i-ATRASO] = (-(float)adc_read() + 1585.0)*0.021055;
        }


        filtro_tensao [0] = 0.0;
        filtro_tensao [1] = 0.0;
        filtro_corrente [0] = 0.0;
        filtro_corrente [1] = 0.0;

        float y_1 = 0.0, y_2 = 0.0;  // estados para tensão
        float z_1 = 0.0, z_2 = 0.0;  // estados para corrente

        for (int i = 0; i < AMOSTRAS; i++) {
            float y, z;

            if (i < 2) {
                y = 0.0;
                z = 0.0;
            } else {
                // Filtro tensão
                y = -X2*y_1 - X3*y_2
                    + X1*leitura_tensao[i]
                    + 2*X1*leitura_tensao[i-1]
                    + X1*leitura_tensao[i-2];

                // Filtro corrente
                z = -X2*z_1 - X3*z_2
                    + X1*leitura_corrente[i]
                    + 2*X1*leitura_corrente[i-1]
                    + X1*leitura_corrente[i-2];
            }

            // Só grava a segunda metade
            if (i >= AMOSTRAS/2) {
                filtro_tensao[i - AMOSTRAS/2] = y;
                filtro_corrente[i - AMOSTRAS/2] = z;
            }

            // Atualiza estados
            y_2 = y_1;  y_1 = y;
            z_2 = z_1;  z_1 = z;
        }

        double soma_v = 0.0, soma_i = 0.0, soma_p = 0.0;

        for(int i = 0; i < AMOSTRAS/2; i++){
            soma_v += filtro_tensao[i] * filtro_tensao[i];
            soma_i += filtro_corrente[i] * filtro_corrente[i];
            soma_p += filtro_tensao[i] * filtro_corrente[i];
        }

        float V_rms = sqrtf(soma_v / (AMOSTRAS/2));
        float I_rms = sqrtf(soma_i / (AMOSTRAS/2));
        float P_media = soma_p / (AMOSTRAS/2);
        float P_aparente = I_rms*V_rms;
        float fp = P_media/P_aparente;

        printf("\n\nTensao RMS: %.2fV\nCorrente RMS: %.2fA\nPotencia Real: %.2fW\nPotencia aparente: %.2fVA\nFator de Potencia: %.2f",
                V_rms, I_rms, P_media, P_aparente, fp);

        sleep_ms(4000);

    }
}