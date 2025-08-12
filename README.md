# Leitor de ADC e Envio via UART – Raspberry Pi Pico

Este programa realiza leituras analógicas de tensão e corrente usando o ADC do Raspberry Pi Pico e envia os valores calculados (RMS) pela UART.

## Funcionalidades

- Leitura de dois canais ADC:
  - GPIO 27 (Tensão)
  - GPIO 26 (Corrente)
- Cálculo RMS a partir de 4000 amostras por ciclo.
- Calibração por fator multiplicativo para conversão em unidades físicas.
- Envio dos valores via UART no formato: T<valor_tensao> C<valor_corrente>
- Intervalo de envio: 500 ms.

## Requisitos

- Raspberry Pi Pico
- Circuito de medição com sensores:
- ZMPT101B (Tensão)
- ACS712 (Corrente)
- Conexão UART com outro dispositivo (ex.: Pico W ou computador)

