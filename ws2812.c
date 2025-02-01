/*
 * Por: Paulo Cesar de Jesus Di Lauro
 *
 * Este código é uma adaptação do código original do
 * professor Wilton Lacerda Silva
 * para a utilização da matriz de LEDs WS2812 do BitDogLab.
 *
 * A seleção de LEDs acesos é feita por meio de um buffer de LEDs, onde
 * cada posição do buffer representa um LED da matriz 5x5.
 *
 * Original em:
 * https://github.com/wiltonlacerda/EmbarcaTechU4C4/tree/72b3f5e897974c0957e3d281d8b548a674a1948d/06_ws2812_Escolha
 */

#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "ws2812.pio.h"

#define IS_RGBW false
#define NUM_PIXELS 25
#define WS2812_PIN 7
#define tempo 200 // para led vermelho piscar 5 vezes por segundo
#define Frames 10
#define LED_PIN_R 13
#define Botao_A 5 // gpio do botão A na BitDogLab
#define Botao_B 6 // gpio do botão B na BitDogLab

// Variável global para armazenar a cor (Entre 0 e 255 para intensidade)
uint8_t led_r = 0;  // Intensidade do vermelho
uint8_t led_g = 25; // Intensidade do verde
uint8_t led_b = 25; // Intensidade do azul

static volatile int aux = 5; // posição do numero impresso na matriz, inicialmente imprime numero 5
// Buffer para armazenar quais LEDs estão ligados matriz 5x5
bool led_buffer[NUM_PIXELS] = {
    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0};
/*posição na matriz de leds para o buffer acima para auxiliar na visualização
    0, 1, 2, 3, 4,
    5, 6, 7, 8, 9,
    10, 11, 12, 13, 14,
    15, 16, 17, 18, 19,
    20, 21, 22, 23, 24
*/

// cotem os frames que formam os numeros de 0 a 9
int bufer_Numeros[Frames][NUM_PIXELS] =
    {
        //
        {0, 1, 1, 1, 0, 0, 1, 0, 1, 0, 0, 1, 0, 1, 0, 0, 1, 0, 1, 0, 0, 1, 1, 1, 0}, // para o numero zero
        {0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 1, 0}, // para o numero 1
        {0, 1, 1, 1, 0, 0, 1, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 1, 0, 0, 1, 1, 1, 0}, // para o numero 2
        {0, 1, 1, 1, 0, 0, 0, 0, 1, 0, 0, 1, 1, 1, 0, 0, 0, 0, 1, 0, 0, 1, 1, 1, 0}, // para o numero 3
        {0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 1, 1, 0, 0, 1, 0, 1, 0, 0, 1, 0, 1, 0}, // para o numero 4
        {0, 1, 1, 1, 0, 0, 0, 0, 1, 0, 0, 1, 1, 1, 0, 0, 1, 0, 0, 0, 0, 1, 1, 1, 0}, // para o numero 5
        {0, 1, 1, 1, 0, 0, 1, 0, 1, 0, 0, 1, 1, 1, 0, 0, 1, 0, 0, 0, 0, 1, 1, 1, 0}, // para o numero 6
        {0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 1, 0, 1, 0, 0, 1, 1, 1, 0}, // para o numero 7
        {0, 1, 1, 1, 0, 0, 1, 0, 1, 0, 0, 1, 1, 1, 0, 0, 1, 0, 1, 0, 0, 1, 1, 1, 0}, // para o numero 8
        {0, 1, 1, 1, 0, 0, 0, 0, 1, 0, 0, 1, 1, 1, 0, 0, 1, 0, 1, 0, 0, 1, 1, 1, 0}  // para o numero 9
};
// função que atualiza o bufer de acordo o numero de 0 a 9
void atualiza_bufer(bool bufer[], int b[][NUM_PIXELS], int c)
{
    for (int i = 0; i < NUM_PIXELS; i++)
    {
        bufer[i] = b[c][i];
    }
}

static inline void put_pixel(uint32_t pixel_grb)
{
    pio_sm_put_blocking(pio0, 0, pixel_grb << 8u);
}

static inline uint32_t urgb_u32(uint8_t r, uint8_t g, uint8_t b)
{
    return ((uint32_t)(r) << 8) | ((uint32_t)(g) << 16) | (uint32_t)(b);
}

void set_one_led(uint8_t r, uint8_t g, uint8_t b)
{
    // Define a cor com base nos parâmetros fornecidos
    uint32_t color = urgb_u32(r, g, b);

    // Define todos os LEDs com a cor especificada
    for (int i = 0; i < NUM_PIXELS; i++)
    {
        if (led_buffer[i])
        {
            put_pixel(color); // Liga o LED com um no buffer
        }
        else
        {
            put_pixel(0); // Desliga os LEDs com zero no buffer
        }
    }
}

// função interrupção para os botões A e B
void gpio_irq_handler(uint gpio, uint32_t events)
{
    if (gpio_get(Botao_A) == 0 && aux < Frames - 1)
    { // se botão A for pressionado e aux<9 incrementa aux em 1(próximo número)
        aux++;
        atualiza_bufer(led_buffer, bufer_Numeros, aux); /// atualiza buffer
        set_one_led(led_r, led_g, led_b);               // forma numero na matriz
    }
    else if (gpio_get(Botao_B) == 0 && aux > 0)
    { // se botão B for pressionado e aux>0 decrementa aux em 1(número anterior)
        aux--;
        atualiza_bufer(led_buffer, bufer_Numeros, aux); // atualiza buffer
        set_one_led(led_r, led_g, led_b);               // forma numero na matriz
    }
}

int main()
{
    PIO pio = pio0;
    int sm = 0;
    uint offset = pio_add_program(pio, &ws2812_program);

    ws2812_program_init(pio, sm, offset, WS2812_PIN, 800000, IS_RGBW);
    // configuração led RGB vermelho
    gpio_init(LED_PIN_R);              // inicializa pino do led vermelho
    gpio_set_dir(LED_PIN_R, GPIO_OUT); // configrua como saída
    gpio_put(LED_PIN_R, 0);            // led inicia apagado

    // configuração botão A
    gpio_init(Botao_A);             // inicializa pino do botão A
    gpio_set_dir(Botao_A, GPIO_IN); // configura como entrada
    gpio_pull_up(Botao_A);          // Habilita o pull-up interno

    // configuração botão B
    gpio_init(Botao_B);             // inicializa pino do botão B
    gpio_set_dir(Botao_B, GPIO_IN); // configura como entrada
    gpio_pull_up(Botao_B);          // Habilita o pull-up interno

    atualiza_bufer(led_buffer, bufer_Numeros, aux); // atualiza buffer para numero 5
    set_one_led(led_r, led_g, led_b);               // forma numero 5 primeira vez

    // configurando da interrupção com botão na descida para o botão A
    gpio_set_irq_enabled_with_callback(Botao_A, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);

    // configurando da interrupção com botão na descida para o botão A
    gpio_set_irq_enabled_with_callback(Botao_B, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);

    while (1)
    {

        gpio_put(LED_PIN_R, 1); // liga led
        sleep_ms(tempo / 2);    // mantem ligado por 100   ms
        gpio_put(LED_PIN_R, 0); // desliga o led
        sleep_ms(tempo / 2);    // mantem desligado por 100   ms, totalizamdo 200 ms de espera até ligar novamente(pisca 5 vezes por segundo)

        // atualiza_bufer(led_buffer,bufer_Numeros, i);
        // set_one_led(led_r, led_g, led_b);
        // set_one_led(0, 0, 0);
    }

    return 0;
}
